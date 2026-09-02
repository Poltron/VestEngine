#include "Systems/Renderer.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Camera.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Managers/ResourcesManager.h"
#include "Resources/Mesh.h"
#include "Resources/Shader.h"

void Renderer::initialize()
{
	glEnable(GL_DEPTH_TEST);
}

void Renderer::setActiveCamera(Camera* inCamera)
{
	activeCamera = inCamera;
}

void Renderer::fillLightParameters(const ComponentManager<WorldTransformComponent>& inWorldTransforms
	, const ComponentManager<PointLightComponent>& inPointLights
	, const ComponentManager<DirectionalLightComponent>& inDirectionalLights)
{
	globalShaderParameters.addVec3("ambientLight.color", { 0.1f, 0.1f, 0.2f });
	globalShaderParameters.addFloat("ambientLight.intensity", 1.0f);

	const DirectionalLightComponent* directionalLightComp = inDirectionalLights.at(0);
	const WorldTransformComponent* directionalLightTransform = inWorldTransforms.get(directionalLightComp->entity);

	//
	globalShaderParameters.addVec3("directionalLight.color", directionalLightComp->color);
	globalShaderParameters.addFloat("directionalLight.intensity", directionalLightComp->intensity);
	globalShaderParameters.addVec3("directionalLight.direction", directionalLightTransform->getModelForward());

	assert(inPointLights.size() <= MAX_POINT_LIGHTS);

	globalShaderParameters.addInt("pointLightAmount", (int)inPointLights.size());

	//
	for (size_t i = 0; i < inPointLights.size(); ++i)
	{
		const PointLightComponent* pointLight = inPointLights.at(i);
		assert(pointLight != nullptr);
		std::string pointLightName = "pointLights[";
		pointLightName.append(std::to_string(i));
		pointLightName.append("]");

		globalShaderParameters.addVec3(pointLightName + ".color", pointLight->color);
		globalShaderParameters.addFloat(pointLightName + ".intensity", pointLight->intensity);
		globalShaderParameters.addFloat(pointLightName + ".constant", pointLight->constant);
		globalShaderParameters.addFloat(pointLightName + ".linear", pointLight->linear); // darken diffuse light a bit
		globalShaderParameters.addFloat(pointLightName + ".quadratic", pointLight->quadratic);

		const WorldTransformComponent* pointLightTransform = inWorldTransforms.get(pointLight->entity);
		assert(pointLightTransform != nullptr);

		globalShaderParameters.addVec3(pointLightName + ".position", pointLightTransform->getPosition());
	}
}

void Renderer::clear()
{
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(ResourcesManager& inResourcesManager
	, ComponentManager<WorldTransformComponent>& inWorldTransforms
	, ComponentManager<MeshRendererComponent>& inMeshRenderers
	, double inCurrentFrame)
{
	if (!activeCamera)
	{
		std::cout << "ERROR: no active camera" << std::endl;
		return;
	}

	for (size_t i = 0; i < inMeshRenderers.size(); ++i)
	{
		MeshRendererComponent* meshRenderer = inMeshRenderers.at(i);
		assert(meshRenderer != nullptr);
		if (!meshRenderer->shader.IsValid())
		{
			std::cout << "WARNING: " << meshRenderer->entity << " has no shader" << std::endl;
			continue;
		}

		Shader* shader = inResourcesManager.getShader(meshRenderer->shader);
		assert(shader != nullptr);

		shader->use();

		//
		globalShaderParameters.applyToShader(*shader, inResourcesManager);

		// 
		glm::mat4& viewMatrix = activeCamera->getViewMatrix();
		shader->setMat4("view", glm::value_ptr(viewMatrix));
		shader->setVec3("viewPosition", activeCamera->getPosition());
		glm::mat4& projectionMatrix = activeCamera->getProjectionMatrix();
		shader->setMat4("projection", glm::value_ptr(projectionMatrix));

		WorldTransformComponent* worldTransform = inWorldTransforms.get(meshRenderer->entity);
		assert(worldTransform != nullptr);

		shader->setMat4("model", glm::value_ptr(worldTransform->model));

		// 
		Model* model = inResourcesManager.getModel(meshRenderer->model);
		assert(model != nullptr);

		model->bindTextures(inResourcesManager, *shader);

		meshRenderer->shaderParameters.applyToShader(*shader, inResourcesManager);

		shader->setFloat("material.shininess", 32.0f);
		
		model->draw();
	}
}

void Renderer::swap(GLFWwindow* inWindow)
{
	glfwSwapBuffers(inWindow);
}

//GLuint Renderer::createVAO(const Mesh& inMesh)
//{
//	GLuint VAO;
//	glGenVertexArrays(1, &VAO);
//	glBindVertexArray(VAO);
//
//	GLuint VBO;
//	glGenBuffers(1, &VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * inMesh.vertices.size(), inMesh.vertices.data(), GL_STATIC_DRAW);
//
//	// info : vertex = x y z u v
//	GLuint vertexAttribPointer = 0; // vertex layout location in vertex shader
//	GLsizei stride = 8 * sizeof(float);
//	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
//	glEnableVertexAttribArray(vertexAttribPointer);
//
//	vertexAttribPointer = 1; // normal layout location in vertex shader
//	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(vertexAttribPointer);
//
//	vertexAttribPointer = 2; // texcoord layout location in vertex shader
//	glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
//	glEnableVertexAttribArray(vertexAttribPointer);
//
//	glBindVertexArray(0);
//
//	return VAO;
//}