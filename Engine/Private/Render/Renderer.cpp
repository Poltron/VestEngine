#include "Render/Renderer.h"

#include <iostream>
#include <algorithm>

#include "glm/gtc/type_ptr.hpp"
#include "tracy/Tracy.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/Ensure.h"
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

void Renderer::render(Scene& inScene, double inDeltaTime)
{
	ZoneScopedN("Renderer::render");

	if (!activeCamera)
	{
		std::cout << "ERROR: no active camera" << std::endl;
		return;
	}
	
	std::vector<RenderedItem> renderedItems;
	frustumCullingSystem.update(inScene, inDeltaTime, renderedItems);
	sortMainPass(inScene, renderedItems);
	recordMainPass(inScene, renderedItems);

	debugShapeSubRenderer.updateDebugShapes(inScene, inDeltaTime);
	recordDebugPass(inScene);
}

void Renderer::sortMainPass(Scene& inScene, std::vector<RenderedItem>& inOutRenderedItems)
{
	const ComponentManager<MeshRendererComponent>& meshRendererComponents = engine::getScene()->getMeshRendererComponents();
	for (size_t i = 0; i < inOutRenderedItems.size(); ++i)
	{
		RenderedItem& renderedItem = inOutRenderedItems[i];

		const MeshRendererComponent* meshRenderer = meshRendererComponents.get(renderedItem.entity);
		ensure(meshRenderer);
		MaterialInstance* materialInstance = engine::getResources()->getMaterialInstance(meshRenderer->materialInstance);
		ensure(materialInstance);

		renderedItem.material = materialInstance->getProxyHandle();

		Model* model = engine::getResources()->getModel(meshRenderer->model);
		ensure(model);
		renderedItem.mesh = model->getMeshes()[0].getProxyHandle();
	}

	std::sort(inOutRenderedItems.begin(), inOutRenderedItems.end(),
		[](const RenderedItem& inA, const RenderedItem& inB)
		{
			return inA.material.id < inB.material.id;
		});
}

void Renderer::recordMainPass(Scene& inScene, const std::vector<RenderedItem>& inRenderedItems)
{
	{
		CommandHeader startHeader;
		startHeader.type = ECommandType::StartRenderpass;
		startHeader.size = sizeof(StartRenderpassCommand);

		StartRenderpassCommand startCommand;
		startCommand.passId = 0; // todo: remove magic number
		render::getCommandRecorder()->add(&startHeader, &startCommand);
	}

	GPUProxyHandle boundMat = GPUProxyHandle::invalid;

	const ComponentManager<MeshRendererComponent>& meshRendererComponents = engine::getScene()->getMeshRendererComponents();
	for (size_t i = 0; i < inRenderedItems.size(); ++i)
	{
		const RenderedItem& renderedItem = inRenderedItems[i];

		if (boundMat != renderedItem.material)
		{
			CommandHeader bindMatHeader;
			bindMatHeader.type = ECommandType::BindMaterial;
			bindMatHeader.size = sizeof(BindMaterialCommand);

			BindMaterialCommand bindMatCommand;
			bindMatCommand.material = renderedItem.material;
			render::getCommandRecorder()->add(&bindMatHeader, &bindMatCommand);
			frameInfo.bindMat++;

			boundMat = renderedItem.material;
		}

		const WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(renderedItem.entity);
		ensure(worldTransform);

		CommandHeader drawHeader;
		drawHeader.type = ECommandType::DrawIndexed;
		drawHeader.size = sizeof(DrawIndexCommand);

		DrawIndexCommand drawCommand;
		drawCommand.model = worldTransform->model;
		drawCommand.mesh = renderedItem.mesh;
		render::getCommandRecorder()->add(&drawHeader, &drawCommand);
		frameInfo.drawIndexed++;
	}

	{
		CommandHeader header;
		header.type = ECommandType::EndRenderpass;
		header.size = sizeof(EndRenderpassCommand);

		EndRenderpassCommand endCommand;
		endCommand.passId = 0; // todo: remove magic number
		render::getCommandRecorder()->add(&header, &endCommand);
	}

	render::getRenderBackend()->render(inScene);
}

void Renderer::recordDebugPass(Scene& inScene)
{
	//debugShapeSubRenderer.render(*this, frameInfo);
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

const Camera& Renderer::getActiveCamera() const
{
	ensure(activeCamera);
	return *activeCamera;
}

void Renderer::loadDefaultMaterials()
{
	ZoneScoped;
	std::cout << "Loading default materials.." << std::endl;

	//debugShapeSubRenderer.loadShaders();
	//debugShapeSubRenderer.loadDebugShapes();

	//std::string renderBackendName = render::getRenderBackend()->getName();
	//const std::string vertexPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/default.vert";
	//const std::string litFragmentPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/lit.frag";
	//GPUAllocHandle litShader = render::getRenderBackend()->compileShader(vertexPath, litFragmentPath);
	//g_Renderer->setLitShader(litShader);
	
	const ShaderParameters& shaderParameters = shader::getUnlitMaterialShaderParameters();
	ResourceHandle unlitMatHandle = engine::getResources()->createMaterial(EMaterialBlendMode::OPAQUE, EMaterialType::UNLIT, shaderParameters);
	const Material* material = engine::getResources()->getMaterial(unlitMatHandle);

	ResourceHandle defaultUnlitMatInstanceHandle = engine::getResources()->createMaterialInstance(material, unlitMatHandle);
	MaterialInstance* unlitMatInstance = engine::getResources()->getMaterialInstance(defaultUnlitMatInstanceHandle);

	ResourceHandle placeholderDiffuse = engine::getResources()->loadTexture(File::getResourcesDir() + "Textures/placeholder.png", "diffuse");
	unlitMatInstance->setTexture("diffuse", placeholderDiffuse);
	unlitMatInstance->setVec4("color", glm::vec4(color::red, 1.0));

	g_Renderer->setUnlitMaterial(unlitMatHandle);

	//const std::string solidColorFragmentPath = File::getProjectDir() + "Resources/Shaders/" + renderBackendName + "/color.frag";
	//GPUAllocHandle solidColorShader = render::getRenderBackend()->compileShader(vertexPath, solidColorFragmentPath);
	//g_Renderer->setSolidColorShader(solidColorShader);

	std::cout << "Loaded default shaders successfully." << std::endl;
}

void Renderer::updateDebugShapes(Scene& inScene, double inDeltaTime)
{
	debugShapeSubRenderer.updateDebugShapes(inScene, inDeltaTime);
}

GPUProxyHandle Renderer::createMaterialProxy(void* inData, uint32_t inOffset)
{
	materialProxies.emplace_back(inData, inOffset);
	return GPUProxyHandle((uint32_t)materialProxies.size() - 1);
}

void Renderer::destroyMaterialProxy(GPUProxyHandle inProxyHandle)
{
	// todo
}

GPUProxyHandle Renderer::createMeshProxy(const Mesh* inMesh)
{
	meshProxies.emplace_back(inMesh);
	return GPUProxyHandle((uint32_t)meshProxies.size() - 1);
}

void Renderer::destroyMeshProxy(GPUProxyHandle inProxyHandle)
{
	// todo
}

GPUProxyHandle Renderer::createTextureProxy(const Texture* inTexture)
{
	textureProxies.emplace_back(inTexture);
	return GPUProxyHandle((uint32_t)textureProxies.size() - 1);
}

void Renderer::destroyTextureProxy(GPUProxyHandle inProxyHandle)
{
	// todo
}

void Renderer::updateLightParameters(Scene& inScene)
{
	ZoneScoped;

	//globalShaderParameters.addVec3("ambientLight.color", { 1.0f, 1.0f, 1.0f });
	//globalShaderParameters.addFloat("ambientLight.intensity", 0.3f);

	//if (inScene.getDirectionalLightComponents().size() > 0)
	//{
	//	const DirectionalLightComponent* directionalLightComp = inScene.getDirectionalLightComponents().at(0);
	//	const WorldTransformComponent* directionalLightTransform = inScene.getWorldTransformComponents().get(directionalLightComp->entity);

	//	//
	//	globalShaderParameters.addVec3("directionalLight.color", directionalLightComp->color);
	//	globalShaderParameters.addFloat("directionalLight.intensity", directionalLightComp->intensity);
	//	globalShaderParameters.addVec3("directionalLight.direction", maths::getMatrixForward(directionalLightTransform->model));
	//}

	//if (inScene.getPointLightComponents().size() > 0)
	//{
	//	assert(inScene.getPointLightComponents().size() <= MAX_POINT_LIGHTS);

	//	globalShaderParameters.addInt("pointLightAmount", (int)inScene.getPointLightComponents().size());

	//	//
	//	for (size_t i = 0; i < inScene.getPointLightComponents().size(); ++i)
	//	{
	//		const PointLightComponent* pointLight = inScene.getPointLightComponents().at(i);
	//		assert(pointLight != nullptr);
	//		std::string pointLightName = "pointLights[";
	//		pointLightName.append(std::to_string(i));
	//		pointLightName.append("]");

	//		globalShaderParameters.addVec3(pointLightName + ".color", pointLight->color);
	//		globalShaderParameters.addFloat(pointLightName + ".intensity", pointLight->intensity);
	//		globalShaderParameters.addFloat(pointLightName + ".constant", pointLight->constant);
	//		globalShaderParameters.addFloat(pointLightName + ".linear", pointLight->linear); // darken diffuse light a bit
	//		globalShaderParameters.addFloat(pointLightName + ".quadratic", pointLight->quadratic);

	//		const WorldTransformComponent* pointLightTransform = inScene.getWorldTransformComponents().get(pointLight->entity);
	//		assert(pointLightTransform != nullptr);

	//		globalShaderParameters.addVec3(pointLightName + ".position", maths::getMatrixTranslation(pointLightTransform->model));
	//	}
	//}
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