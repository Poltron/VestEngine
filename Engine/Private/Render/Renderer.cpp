#include "Render/Renderer.h"

#include <iostream>
#include <algorithm>

#include "glm/gtc/type_ptr.hpp"
#include "tracy/Tracy.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/FileSystem.h"
#include "Core/Maths.h"
#include "Core/Scene.h"
#include "Core/ResourcesManager.h"
#include "Core/Resources/Model.h"
#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/SphereColliderComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ComponentManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "Render/Color.h"
#include "Render/Resources/Mesh.h"
#include "RenderBackend/RenderBackend.h"

class VestRenderer final : public Renderer
{
public:
	bool initialize();
	void shutdown();
};

namespace render
{
	VestRenderer* g_Renderer = nullptr;
}
using namespace render;

//
bool VestRenderer::initialize()
{
	return true;
}

void VestRenderer::shutdown()
{}

//
void Renderer::clear()
{
	render::getRenderBackend()->clear();

	frameInfo.reset();
}

void Renderer::render(Scene& inScene)
{
	ZoneScopedN("Renderer::render");

	if (!activeCamera)
	{
		std::cout << "ERROR: no active camera" << std::endl;
		return;
	}

	renderMainPass(inScene);
	renderDebugPass(inScene);
}

void Renderer::renderMainPass(Scene& inScene)
{
	std::sort(inScene.getMeshRendererComponents().getData()
		, inScene.getMeshRendererComponents().getData() + inScene.getMeshRendererComponents().size()
		, [](const MeshRendererComponent& inA, const MeshRendererComponent& inB)
		{
			return inA.shader.id < inB.shader.id;
		});
	inScene.getMeshRendererComponents().rebuildLookupTable();

	render::getRenderBackend()->drawGeometry(inScene);
}

void Renderer::renderDebugPass(Scene& inScene)
{
	debugShapeSubRenderer.render(*this, frameInfo);
}

void Renderer::swap()
{
	platform::getWindowManager().swapBuffers();
	render::getRenderBackend()->tracyGPUCollect();
}

void Renderer::setActiveCamera(Camera* inCamera)
{
	if (activeCamera)
	{
		activeCamera->onCameraSelected(false);
	}

	activeCamera = inCamera;

	if (activeCamera)
	{
		activeCamera->onCameraSelected(true);
	}
}

Camera& Renderer::getActiveCamera()
{
	ensure(activeCamera);
	return *activeCamera;
}

void Renderer::drawModel(const Model& inModel)
{
	const auto& meshes = inModel.getMeshes();
	for (size_t meshIndex = 0; meshIndex < inModel.getMeshes().size(); ++meshIndex)
	{
		getRenderBackend()->drawMesh(meshes[meshIndex]);
		frameInfo.drawCalls++;
	}
}

void Renderer::loadDefaultShaders()
{
	ZoneScoped;
	std::cout << "Loading default shaders.." << std::endl;
	debugShapeSubRenderer.loadShaders();
	debugShapeSubRenderer.loadDebugShapes();
	
	std::string renderBackendName = render::getRenderBackend()->getName();
	const std::string vertexPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/default.vert";
	const std::string litFragmentPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/lit.frag";
	GPUHandle litShader = render::getRenderBackend()->compileShader(vertexPath, litFragmentPath);
	g_Renderer->setLitShader(litShader);

	const std::string unlitFragmentPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/unlit.frag";
	GPUHandle unlitShader = render::getRenderBackend()->compileShader(vertexPath, unlitFragmentPath);
	g_Renderer->setUnlitShader(unlitShader);

	const std::string solidColorFragmentPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/color.frag";
	GPUHandle solidColorShader = render::getRenderBackend()->compileShader(vertexPath, solidColorFragmentPath);
	g_Renderer->setSolidColorShader(solidColorShader);

	std::cout << "Loaded default shaders successfully." << std::endl;
}

void Renderer::updateDebugShapes(Scene& inScene)
{
	debugShapeSubRenderer.updateDebugShapes(inScene);
}

void Renderer::updateLightParameters(Scene& inScene)
{
	ZoneScoped;

	globalShaderParameters.addVec3("ambientLight.color", { 1.0f, 1.0f, 1.0f });
	globalShaderParameters.addFloat("ambientLight.intensity", 0.3f);

	if (inScene.getDirectionalLightComponents().size() > 0)
	{
		const DirectionalLightComponent* directionalLightComp = inScene.getDirectionalLightComponents().at(0);
		const WorldTransformComponent* directionalLightTransform = inScene.getWorldTransformComponents().get(directionalLightComp->entity);

		//
		globalShaderParameters.addVec3("directionalLight.color", directionalLightComp->color);
		globalShaderParameters.addFloat("directionalLight.intensity", directionalLightComp->intensity);
		globalShaderParameters.addVec3("directionalLight.direction", maths::getMatrixForward(directionalLightTransform->model));
	}

	if (inScene.getPointLightComponents().size() > 0)
	{
		assert(inScene.getPointLightComponents().size() <= MAX_POINT_LIGHTS);

		globalShaderParameters.addInt("pointLightAmount", (int)inScene.getPointLightComponents().size());

		//
		for (size_t i = 0; i < inScene.getPointLightComponents().size(); ++i)
		{
			const PointLightComponent* pointLight = inScene.getPointLightComponents().at(i);
			assert(pointLight != nullptr);
			std::string pointLightName = "pointLights[";
			pointLightName.append(std::to_string(i));
			pointLightName.append("]");

			globalShaderParameters.addVec3(pointLightName + ".color", pointLight->color);
			globalShaderParameters.addFloat(pointLightName + ".intensity", pointLight->intensity);
			globalShaderParameters.addFloat(pointLightName + ".constant", pointLight->constant);
			globalShaderParameters.addFloat(pointLightName + ".linear", pointLight->linear); // darken diffuse light a bit
			globalShaderParameters.addFloat(pointLightName + ".quadratic", pointLight->quadratic);

			const WorldTransformComponent* pointLightTransform = inScene.getWorldTransformComponents().get(pointLight->entity);
			assert(pointLightTransform != nullptr);

			globalShaderParameters.addVec3(pointLightName + ".position", maths::getMatrixTranslation(pointLightTransform->model));
		}
	}
}

//
namespace render
{
	Renderer* getRenderer() 
	{ 
		return g_Renderer;
	}

	bool initialize()
	{
		ZoneScoped;

		ensure(!g_Renderer);
		g_Renderer = new VestRenderer();
		bool bSuccess = g_Renderer->initialize();
		if (bSuccess)
		{
			std::cout << "Renderer initialization success." << std::endl;
		}
		else
		{
			std::cout << "Renderer initialization failed." << std::endl;
		}
		return bSuccess;
	}

	void shutdown()
	{
		ZoneScoped;

		if (g_Renderer)
		{
			g_Renderer->shutdown();
		}
		delete g_Renderer;
		g_Renderer = nullptr;
	}
}