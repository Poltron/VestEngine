#include "Render/Renderer.h"

#include <iostream>

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "Camera.h"
#include "Core/Engine.h"
#include "Core/Maths.h"
#include "Core/Scene.h"
#include "Core/ResourcesManager.h"
#include "Core/Resources/Mesh.h"
#include "Core/Resources/Shader.h"
#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/SphereColliderComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ComponentManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "Render/Color.h"
#include "Render/DebugShapes.h"
#include "Render/DebugShapeMeshGenerationHelper.h"

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
	loadDebugShapes();

	glEnable(GL_PROGRAM_POINT_SIZE);

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

//
//
namespace
{
	enum class EDebugShapePrimitive : unsigned int
	{
		POINT = 0,
		LINE,
		ENUM_SIZE
	};

	enum class EDebugShapeMesh : unsigned int
	{
		BOX = 0,
		SPHERE,
		ENUM_SIZE
	};
}

void Renderer::clear()
{
	ZoneScoped;
	TracyGpuZone("Renderer::clear");

	glStencilMask(0xFF);
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	frameInfo.reset();
}

void Renderer::render(Scene& inScene)
{
	ZoneScopedN("Renderer::render");
	TracyGpuZone("Renderer::render");

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

		Shader* shader = nullptr;
		WorldTransformComponent* worldTransform = nullptr;
		{
			shader = engine::getResources()->getShader(meshRenderer->shader);
			ensure(shader != nullptr);
			shader->use();
			frameInfo.shaderPrograms++;

			globalShaderParameters.applyToShader(*shader, *engine::getResources());
		}

		{
			shader->setVec3("viewPosition", activeCamera->getPosition());
			shader->setMat4("viewProjection", glm::value_ptr(activeCamera->getViewProjectionMatrix()));

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

			frameInfo.drawCalls++;
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
			frameInfo.shaderPrograms++;

			glm::mat4 outlineMat = worldTransform->model;
			outlineMat = glm::scale(outlineMat, glm::vec3(1.1f, 1.1f, 1.1f));

			shader->setMat4("model", glm::value_ptr(outlineMat));
			shader->setVec3("objectColor", color::yellow);

			frameInfo.drawCalls++;
			model->draw();

			glStencilFunc(GL_ALWAYS, 1, 0xFF); // every fragment passes stencil
		}
	}
}

void Renderer::renderDebugPass(Scene& inScene)
{
	Shader* shader = engine::getResources()->getShader(debugPrimitiveShaderHandle);
	ensure(shader != nullptr);
	shader->use();
	frameInfo.shaderPrograms++;

	glm::mat4 viewProjection = activeCamera->getProjectionMatrix() * activeCamera->getViewMatrix();
	shader->setMat4("uViewProjection", glm::value_ptr(viewProjection));

	{
		ZoneScopedN("render point primitives");

		DebugShapePrimitive& pointPrimitive = debugShapePrimitives[(unsigned int)EDebugShapePrimitive::POINT];
		glBindBuffer(GL_ARRAY_BUFFER, pointPrimitive.getVBO());
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(pointDebugShapeInstances.size() * sizeof(DebugShapePrimitiveInstance)), pointDebugShapeInstances.data());

		glBindVertexArray(pointPrimitive.getVAO());
		glDrawArrays(GL_POINTS, 0, (GLsizei)(pointDebugShapeInstances.size()));
		glBindVertexArray(0);

		frameInfo.drawCalls++;
		frameInfo.debugShapesTotal += pointDebugShapeInstances.size();
	}

	{
		ZoneScopedN("render line primitives");

		DebugShapePrimitive& linePrimitive = debugShapePrimitives[(unsigned int)EDebugShapePrimitive::LINE];
		glBindBuffer(GL_ARRAY_BUFFER, linePrimitive.getVBO());
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(lineDebugShapeInstances.size() * sizeof(DebugShapePrimitiveInstance)), lineDebugShapeInstances.data());

		glBindVertexArray(linePrimitive.getVAO());
		glDrawArrays(GL_LINES, 0, (GLsizei)(lineDebugShapeInstances.size()));
		glBindVertexArray(0);

		frameInfo.drawCalls++;
		frameInfo.debugShapesTotal += lineDebugShapeInstances.size() / 2;
	}

	shader = engine::getResources()->getShader(debugMeshShaderHandle);
	ensure(shader != nullptr);
	shader->use();
	frameInfo.shaderPrograms++;

	glm::mat4 viewProjection2 = activeCamera->getProjectionMatrix() * activeCamera->getViewMatrix();
	shader->setMat4("uViewProjection", glm::value_ptr(viewProjection2));

	{
		ZoneScopedN("render box meshes");

		DebugShapeMesh& boxMesh = debugShapeMeshes[(unsigned int)EDebugShapeMesh::BOX];
		glBindBuffer(GL_ARRAY_BUFFER, boxMesh.getVBOInstances());
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(boxDebugShapeInstances.size() * sizeof(DebugShapeMeshInstance)), boxDebugShapeInstances.data());

		glBindVertexArray(boxMesh.getVAO());
		glDrawArraysInstanced(GL_LINES, 0, (GLsizei)(boxDebugShapeInstances.size() * sizeof(DebugShapeMeshInstance)), (GLsizei)boxDebugShapeInstances.size());
		glBindVertexArray(0);

		frameInfo.drawCalls++;
		frameInfo.debugShapesTotal += boxDebugShapeInstances.size();
	}

	{
		ZoneScopedN("render sphere meshes");

		DebugShapeMesh& sphereMesh = debugShapeMeshes[(unsigned int)EDebugShapeMesh::SPHERE];
		glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.getVBOInstances());
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(sphereDebugShapeInstances.size() * sizeof(DebugShapeMeshInstance)), sphereDebugShapeInstances.data());

		glBindVertexArray(sphereMesh.getVAO());
		glDrawArraysInstanced(GL_LINES, 0, (GLsizei)(sphereDebugShapeInstances.size() * sizeof(DebugShapeMeshInstance)), (GLsizei)sphereDebugShapeInstances.size());
		glBindVertexArray(0);

		frameInfo.drawCalls++;
		frameInfo.debugShapesTotal += sphereDebugShapeInstances.size();
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

void Renderer::addPoint(const glm::vec3& inPosition, float inSize, const glm::vec3& inColor)
{
	DebugShapePrimitiveInstance pointInstance;
	pointInstance.color = inColor;
	pointInstance.position = inPosition;
	pointInstance.size = inSize;
	pointDebugShapeInstances.push_back(pointInstance);
}

void Renderer::addLine(const glm::vec3& inStart, const glm::vec3& inEnd, float inSize, const glm::vec3& inColor)
{
	DebugShapePrimitiveInstance lineStartInstance;
	lineStartInstance.color = inColor;
	lineStartInstance.position = inStart;
	lineStartInstance.size = inSize;
	lineDebugShapeInstances.push_back(lineStartInstance);

	DebugShapePrimitiveInstance lineEndInstance;
	lineEndInstance.color = inColor;
	lineEndInstance.position = inEnd;
	lineEndInstance.size = inSize;
	lineDebugShapeInstances.push_back(lineEndInstance);
}

void Renderer::addSphere(const glm::vec3& inPosition, const glm::vec3& inRotation, float inRadius, const glm::vec3& inColor)
{
	DebugShapeMeshInstance sphereInstance;
	sphereInstance.color = inColor;
	maths::computeTransformMatrix(inPosition, inRotation, glm::vec3(inRadius), sphereInstance.model);
	sphereDebugShapeInstances.push_back(sphereInstance);
}

void Renderer::addBox(const glm::vec3& inPosition, const glm::vec3& inRotation, const glm::vec3& inScale, const glm::vec3& inColor)
{
	DebugShapeMeshInstance boxInstance;
	boxInstance.color = inColor;
	maths::computeTransformMatrix(inPosition, inRotation, inScale, boxInstance.model);
	boxDebugShapeInstances.push_back(boxInstance);
}

void Renderer::loadDefaultShaders()
{
	ZoneScoped;

	const std::string WorkDirTMP = WORKDIR;

	const std::string vertexPath = WorkDirTMP + "/Resources/Shaders/default.vert";
	const std::string litFragmentPath = WorkDirTMP + "/Resources/Shaders/lit.frag";
	ResourceHandle litShader = engine::getResources()->loadShader(vertexPath, litFragmentPath);
	g_Renderer->setLitShader(litShader);

	const std::string unlitFragmentPath = WorkDirTMP + "/Resources/Shaders/unlit.frag";
	ResourceHandle unlitShader = engine::getResources()->loadShader(vertexPath, unlitFragmentPath);
	g_Renderer->setUnlitShader(unlitShader);

	const std::string solidColorFragmentPath = WorkDirTMP + "/Resources/Shaders/color.frag";
	ResourceHandle solidColorShader = engine::getResources()->loadShader(vertexPath, solidColorFragmentPath);
	g_Renderer->setSolidColorShader(solidColorShader);

	const std::string debugMeshVertexPath = WorkDirTMP + "/Resources/Shaders/debug_mesh.vert";
	const std::string debugFragmentPath = WorkDirTMP + "/Resources/Shaders/debug.frag";
	ResourceHandle debugMeshShader = engine::getResources()->loadShader(debugMeshVertexPath, debugFragmentPath);
	g_Renderer->setDebugMeshShader(debugMeshShader);

	const std::string debugPrimitiveVertexPath = WorkDirTMP + "/Resources/Shaders/debug_primitive.vert";
	ResourceHandle debugPrimitiveShader = engine::getResources()->loadShader(debugPrimitiveVertexPath, debugFragmentPath);
	g_Renderer->setDebugPrimitiveShader(debugPrimitiveShader);
}

void Renderer::loadDebugShapes()
{
	ZoneScoped;

	debugShapePrimitives.resize((size_t)EDebugShapePrimitive::ENUM_SIZE);
	// todo: not the cleanest ? right now its following enum order but i can't resize + assign
	// with enum index since there's no default constructor on DebugShapeMesh
	debugShapeMeshes.push_back(debugShapeMeshGenerationHelper::createBox());
	debugShapeMeshes.push_back(debugShapeMeshGenerationHelper::createSphere(6));
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