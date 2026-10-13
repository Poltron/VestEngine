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
#include "Core/Resources/Mesh.h"
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

	for (size_t i = 0; i < inScene.getMeshRendererComponents().size(); ++i)
	{
		ZoneScoped;

		MeshRendererComponent* meshRenderer = inScene.getMeshRendererComponents().at(i);
		ensure(meshRenderer != nullptr);
		if (!meshRenderer->shader.isValid())
		{
			std::cout << "WARNING: " << meshRenderer->entity << " has no shader" << std::endl;
			continue;
		}

		SphereColliderComponent* sphere = inScene.getSphereColliderComponents().get(meshRenderer->entity);
		ensure(sphere != nullptr);
		if (!sphere->bInFrustum)
		{
			frameInfo.culledMeshTotal++;
			continue;
		}
		else
		{
			frameInfo.renderedMeshTotal++;
		}

		Model* model = engine::getResources()->getModel(meshRenderer->model);
		ensure(model != nullptr);

		const std::string meshRendererName = "render " + model->getPath();
		ZoneName(meshRendererName.c_str(), meshRendererName.size());

		ensure(meshRenderer->shader.isValid());
		render::getRenderBackend()->useShaderProgram(meshRenderer->shader);
		frameInfo.shaderPrograms++;

		globalShaderParameters.applyToShader();

		WorldTransformComponent* worldTransform = nullptr;
		{
			render::getRenderBackend()->setVec3("viewPosition", activeCamera->getPosition());
			render::getRenderBackend()->setMat4("viewProjection", glm::value_ptr(activeCamera->getViewProjectionMatrix()));

			worldTransform = inScene.getWorldTransformComponents().get(meshRenderer->entity);
			ensure(worldTransform != nullptr);

			render::getRenderBackend()->setMat4("model", glm::value_ptr(worldTransform->model));
		}

		{
			ensure(model != nullptr);
			model->bindTextures(*engine::getResources());
		}

		{
			meshRenderer->shaderParameters.applyToShader();
			render::getRenderBackend()->setFloat("material.shininess", 32.0f);
		}

		{
			if (meshRenderer->bOutline)
			{
				render::getRenderBackend()->setStencilMask(0xFF); // allow full writing to stencil
			}

			drawModel(*model);
		}

		if (meshRenderer->bOutline)
		{
			render::getRenderBackend()->setStencilFunc(EComparisonFunc::NOTEQUAL, 1, 0xFF);
			render::getRenderBackend()->setStencilMask(0x00); // don't write to stencil

			ensure(getSolidColorShader().isValid());
			render::getRenderBackend()->useShaderProgram(getSolidColorShader());
			frameInfo.shaderPrograms++;

			glm::mat4 outlineMat = worldTransform->model;
			outlineMat = glm::scale(outlineMat, glm::vec3(1.5f, 1.5f, 1.5f));

			render::getRenderBackend()->setMat4("model", glm::value_ptr(outlineMat));
			render::getRenderBackend()->setVec3("objectColor", color::yellow);

			drawModel(*model);

			render::getRenderBackend()->setStencilFunc(EComparisonFunc::ALWAYS, 1, 0xFF); // every fragment passes stencil
		}
	}
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
	for (size_t meshIndex = 0; meshIndex < inModel.getMeshes().size(); ++meshIndex)
	{
		const Mesh& mesh = inModel.getMeshes()[meshIndex];
		if (mesh.getEBO().id)
		{
			render::getRenderBackend()->drawElements(mesh.getVAO(), EPrimitiveMode::TRIANGLES, mesh.getIndicesSize(), 0);
			frameInfo.drawCalls++;
		}
		else
		{
			render::getRenderBackend()->drawArrays(mesh.getVAO(), EPrimitiveMode::TRIANGLES, 0, mesh.getVerticesSize());
			frameInfo.drawCalls++;
		}
	}
}

void Renderer::loadDefaultShaders()
{
	ZoneScoped;

	debugShapeSubRenderer.loadShaders();
	debugShapeSubRenderer.loadDebugShapes();

	const std::string vertexPath = File::getProjectDir() + "Resources/Shaders/OpenGL/default.vert";
	const std::string litFragmentPath = File::getProjectDir() + "Resources/Shaders/OpenGL/lit.frag";
	GPUHandle litShader = render::getRenderBackend()->compileShader(vertexPath, litFragmentPath);
	g_Renderer->setLitShader(litShader);

	const std::string unlitFragmentPath = File::getProjectDir() + "Resources/Shaders/OpenGL/unlit.frag";
	GPUHandle unlitShader = render::getRenderBackend()->compileShader(vertexPath, unlitFragmentPath);
	g_Renderer->setUnlitShader(unlitShader);

	const std::string solidColorFragmentPath = File::getProjectDir() + "Resources/Shaders/OpenGL/color.frag";
	GPUHandle solidColorShader = render::getRenderBackend()->compileShader(vertexPath, solidColorFragmentPath);
	g_Renderer->setSolidColorShader(solidColorShader);
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