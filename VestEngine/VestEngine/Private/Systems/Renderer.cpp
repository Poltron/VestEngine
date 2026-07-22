#include "Systems/Renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Camera.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Resources/Mesh.h"
#include "Resources/Shader.h"

#include <iostream>

void Renderer::setActiveCamera(Camera* inCamera)
{
	activeCamera = inCamera;
}

void Renderer::clear()
{
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(ComponentManager<TransformComponent>& inTransforms
	, ComponentManager<MeshRendererComponent>& inMeshRenderers
	, ComponentManager<PointLightComponent>& inPointLights
	, ComponentManager<DirectionalLightComponent>& inDirectionalLights
	, double inCurrentFrame)
{
	if (!activeCamera)
	{
		std::cout << "ERROR: no active camera" << std::endl;
		return;
	}

	DirectionalLightComponent* directionalLightComp = inDirectionalLights.at(0);
	TransformComponent* directionalLightTransform = inTransforms.get(directionalLightComp->entityID);

	for (size_t i = 0; i < inMeshRenderers.size(); ++i)
	{
		MeshRendererComponent* meshRenderer = inMeshRenderers.at(i);
		assert(meshRenderer != nullptr);
		if (meshRenderer->shaderID == 0)
		{
			std::cout << "WARNING: " << meshRenderer->entityID << " has no shader" << std::endl;
			continue;
		}

		Shader* shader = getShader(meshRenderer->shaderID);
		assert(shader != nullptr);

		shader->use();

		if (meshRenderer->texture0ID != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture0ID);

			shader->setInt("material.diffuse", 0);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if (meshRenderer->texture1ID != 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture1ID);

			shader->setInt("material.specular", 1);
		}
		else
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		shader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		shader->setFloat("material.shininess", 32.0f);

		shader->setVec3("objectColor", meshRenderer->objectColor);

		glm::mat4& viewMatrix = activeCamera->getViewMatrix();
		shader->setMat4("view", glm::value_ptr(viewMatrix));
		shader->setVec3("viewPosition", activeCamera->getPosition());
		glm::mat4& projectionMatrix = activeCamera->getProjectionMatrix();
		shader->setMat4("projection", glm::value_ptr(projectionMatrix));

		TransformComponent* transform = inTransforms.get(meshRenderer->entityID);
		assert(transform != nullptr);

		shader->setMat4("model", glm::value_ptr(transform->model));

		//
		shader->setVec3("ambientLight.color", 0.1f, 0.1f, 0.2f);
		shader->setFloat("ambientLight.intensity", 1.0f);

		//
		shader->setVec3("directionalLight.color", directionalLightComp->color);
		shader->setFloat("directionalLight.intensity", directionalLightComp->intensity);
		shader->setVec3("directionalLight.direction", directionalLightTransform->getModelForward());
		
		//
		for (int i = 0; i < inPointLights.size(); ++i)
		{
			PointLightComponent* pointLight = inPointLights.getData() + i;
			assert(pointLight != nullptr);
			std::string pointLightName = "pointLights[";
			pointLightName.append(std::to_string(i));
			pointLightName.append("]");

			shader->setVec3(pointLightName + ".color", pointLight->color);
			shader->setFloat(pointLightName + ".intensity", pointLight->intensity);
			shader->setFloat(pointLightName + "constant", pointLight->constant);
			shader->setFloat(pointLightName + ".linear", pointLight->linear); // darken diffuse light a bit
			shader->setFloat(pointLightName + ".quadratic", pointLight->quadratic);

			TransformComponent* pointLightTransform = inTransforms.get(pointLight->entityID);
			assert(pointLightTransform != nullptr);

			shader->setVec3(pointLightName + ".position", pointLightTransform->position);
		}

		glBindVertexArray(meshRenderer->VAOID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}

void Renderer::swap(GLFWwindow* inWindow)
{
	glfwSwapBuffers(inWindow);
}

GLuint Renderer::addShader(const char* inVertexPath, const char* inFragmentPath)
{
	Shader shader;
	shader.load(inVertexPath, inFragmentPath);

	shaders.push_back(shader);
	return shader.ID;
}

GLuint Renderer::createVAO(const Mesh& inMesh)
{
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * inMesh.vertices.size(), inMesh.vertices.data(), GL_STATIC_DRAW);

	// info : vertex = x y z u v
	GLuint vertexAttribPointer = 0; // vertex layout location in vertex shader
	GLsizei stride = 8 * sizeof(float);
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(vertexAttribPointer);

	vertexAttribPointer = 1; // normal layout location in vertex shader
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vertexAttribPointer);

	vertexAttribPointer = 2; // texcoord layout location in vertex shader
	glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(vertexAttribPointer);

	glBindVertexArray(0);

	return VAO;
}