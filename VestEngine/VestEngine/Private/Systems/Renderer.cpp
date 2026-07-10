#include "Systems/Renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Camera.h"
#include "Components/MeshRendererComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Resources/Mesh.h"
#include "Resources/Shader.h"

void Renderer::clear()
{
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(ComponentManager<TransformComponent>& inTransforms, ComponentManager<MeshRendererComponent>& inMeshRenderers, Camera inCamera, const glm::vec3& inLightPosition, double inCurrentFrame)
{
	for (size_t i = 0; i < inMeshRenderers.size(); ++i)
	{
		MeshRendererComponent* meshRenderer = inMeshRenderers.at(i);
		assert(meshRenderer != nullptr);

		Shader* shader = getShader(meshRenderer->shaderID);
		assert(shader != nullptr);

		shader->use();


		if (meshRenderer->texture0ID != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture0ID);

			shader->setInt("texture0", 0);
		}

		if (meshRenderer->texture1ID != 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture1ID);

			shader->setInt("texture1", 1);
		}

		shader->setVec3("material.ambient", meshRenderer->objectColor);
		shader->setVec3("material.diffuse", meshRenderer->objectColor);
		shader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		shader->setFloat("material.shininess", 32.0f);

		shader->setVec3("light.position", inLightPosition);
		shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f); // darken diffuse light a bit
		shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		shader->setVec3("objectColor", meshRenderer->objectColor);
		shader->setVec3("lightColor", meshRenderer->lightColor);

		glm::mat4& viewMatrix = inCamera.getViewMatrix();
		shader->setMat4("view", glm::value_ptr(viewMatrix));
		shader->setVec3("viewPosition", inCamera.getPosition());
		glm::mat4& projectionMatrix = inCamera.getProjectionMatrix();
		shader->setMat4("projection", glm::value_ptr(projectionMatrix));

		TransformComponent* TransformComponent = inTransforms.get(meshRenderer->entityID);
		shader->setMat4("model", glm::value_ptr(TransformComponent->model));

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

GLuint Renderer::createCubeVAO(const Mesh& inMesh)
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
	GLsizei stride = 6 * sizeof(float);
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(vertexAttribPointer);

	vertexAttribPointer = 1; // normal layout location in vertex shader
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vertexAttribPointer);

	//vertexAttribPointer = 1; // texcoord layout location in vertex shader
	//glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(vertexAttribPointer);

	glBindVertexArray(0);

	return VAO;
}

GLuint Renderer::createLightVAO(const Mesh& inMesh)
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
	GLsizei stride = 6 * sizeof(float);
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(vertexAttribPointer);

	vertexAttribPointer = 1; // normal layout location in vertex shader
	glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vertexAttribPointer);

	//vertexAttribPointer = 1; // vertex layout location in vertex shader
	//glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(vertexAttribPointer);

	glBindVertexArray(0);

	return VAO;
}