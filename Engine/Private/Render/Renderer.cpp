#include "Render/Renderer.h"

#include <iostream>

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Core/ResourcesManager.h"
#include "Core/Resources/Mesh.h"
#include "Core/Resources/Shader.h"
#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ComponentManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

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
	// depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0x00);

	return true;
}

void VestRenderer::shutdown()
{}

void Renderer::clear()
{
	ZoneScoped;
	TracyGpuZone("Renderer::clear");

	glStencilMask(0xFF);
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
}

void Renderer::render(Scene& inScene, double inCurrentFrame)
{
	ZoneScopedN("Renderer::render");
	TracyGpuZone("Renderer::render");

	if (!activeCamera)
	{
		std::cout << "ERROR: no active camera" << std::endl;
		return;
	}

	for (size_t i = 0; i < inScene.getMeshRendererComponents().size(); ++i)
	{
		ZoneScoped;

		MeshRendererComponent* meshRenderer = inScene.getMeshRendererComponents().at(i);
		ensure(meshRenderer != nullptr);
		if (!meshRenderer->shader.IsValid())
		{
			std::cout << "WARNING: " << meshRenderer->entity << " has no shader" << std::endl;
			continue;
		}

		SphereBoundingVolumeComponent* sphere = inScene.getSphereBoundingVolumeComponents().get(meshRenderer->entity);
		ensure(sphere != nullptr);
		if (!sphere->bInFrustum)
		{
			continue;
		}

		Model* model = engine::getResources()->getModel(meshRenderer->model);
		ensure(model != nullptr);
		
		const std::string meshRendererName = "render " + model->getPath();
		ZoneName(meshRendererName.c_str(), meshRendererName.size());

		Shader* shader = nullptr;
		WorldTransformComponent* worldTransform = nullptr;
		{

			shader = engine::getResources()->getShader(meshRenderer->shader);
			ensure(shader != nullptr);
			shader->use();

			globalShaderParameters.applyToShader(*shader, *engine::getResources());
		}

		{
			glm::mat4& viewMatrix = activeCamera->getViewMatrix();
			shader->setMat4("view", glm::value_ptr(viewMatrix));
			shader->setVec3("viewPosition", activeCamera->getPosition());
			glm::mat4& projectionMatrix = activeCamera->getProjectionMatrix();
			shader->setMat4("projection", glm::value_ptr(projectionMatrix));

			worldTransform = inScene.getWorldTransformComponents().get(meshRenderer->entity);
			ensure(worldTransform != nullptr);

			shader->setMat4("model", glm::value_ptr(worldTransform->model));
		}

		{
			ensure(model != nullptr);
			model->bindTextures(*engine::getResources(), *shader);
		}

		{
			meshRenderer->shaderParameters.applyToShader(*shader, *engine::getResources());

			shader->setFloat("material.shininess", 32.0f);
		}

		{
			if (meshRenderer->bOutline)
			{
				glStencilMask(0xFF); // allow full writing to stencil
			}

			model->draw();
		}

		if (meshRenderer->bOutline)
		{
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // every fragment where stencil is not equal to 1 passes
			glStencilMask(0x00); // don't write to stencil

			ensure(getSolidColorShader().IsValid());
			Shader* shader = engine::getResources()->getShader(getSolidColorShader());
			ensure(shader);
			shader->use();

			glm::mat4 outlineMat = worldTransform->model;
			outlineMat = glm::scale(outlineMat, glm::vec3(1.1f, 1.1f, 1.1f));

			shader->setMat4("model", glm::value_ptr(outlineMat));
			shader->setVec3("objectColor", glm::vec3(1.0f, 1.0f, 0.0f));

			model->draw();

			glStencilFunc(GL_ALWAYS, 1, 0xFF); // every fragment passes stencil
		}
	}
}

void Renderer::swap()
{
	{
		ZoneScoped;
		TracyGpuZone("Renderer::swap");
		platform::getWindowManager().swapBuffers();
	}

	TracyGpuCollect;
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

void Renderer::loadDefaultShaders()
{
	ZoneScoped;

	const std::string WorkDirTMP = WORKDIR;

	const std::string vertexPath = WorkDirTMP + "/Resources/Shaders/vertex.glsl";
	const std::string litFragmentPath = WorkDirTMP + "/Resources/Shaders/lit_fragment.glsl";
	ResourceHandle litShader = engine::getResources()->loadShader(vertexPath, litFragmentPath);
	g_Renderer->setLitShader(litShader);

	const std::string unlitFragmentPath = WorkDirTMP + "/Resources/Shaders/unlit_fragment.glsl";
	ResourceHandle unlitShader = engine::getResources()->loadShader(vertexPath, unlitFragmentPath);
	g_Renderer->setUnlitShader(unlitShader);

	const std::string solidColorFragmentPath = WorkDirTMP + "/Resources/Shaders/color_fragment.glsl";
	ResourceHandle solidColorShader = engine::getResources()->loadShader(vertexPath, solidColorFragmentPath);
	g_Renderer->setSolidColorShader(solidColorShader);
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
		globalShaderParameters.addVec3("directionalLight.direction", directionalLightTransform->getModelForward());
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

			globalShaderParameters.addVec3(pointLightName + ".position", pointLightTransform->getPosition());
		}
	}
}

//
Renderer* render::getRenderer() { return g_Renderer; }
bool render::initialize()
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

void render::shutdown()
{
	ZoneScoped;

	if (g_Renderer)
	{
		g_Renderer->shutdown();
	}
	delete g_Renderer;
	g_Renderer = nullptr;
}