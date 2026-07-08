#pragma once

#include "Components/MeshRendererComponent.h"
#include "Resources/Shader.h"

struct GLFWwindow;

class Renderer
{
public:
	void clear()
	{
		glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void render(ComponentManager<TransformComponent>& inTransforms, ComponentManager<MeshRendererComponent>& inMeshRenderers, Camera inCamera, double inCurrentFrame)
	{
		for (size_t i = 0; i < inMeshRenderers.size(); ++i)
		{
			MeshRendererComponent* meshRenderer = inMeshRenderers.at(i);
			assert(meshRenderer != nullptr);

			Shader* shader = getShader(meshRenderer->shaderID);
			assert(shader != nullptr);

			shader->use();

			float alpha = ((float)sin(inCurrentFrame) / 2.0f) + 0.5f;
			shader->setFloat("alpha", alpha);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture0ID);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshRenderer->texture1ID);

			shader->setInt("texture0", 0);
			shader->setInt("texture1", 1);

			glm::mat4& viewMatrix = inCamera.getViewMatrix();
			shader->setMat4("view", glm::value_ptr(viewMatrix));

			glm::mat4& projectionMatrix = inCamera.getProjectionMatrix();
			shader->setMat4("projection", glm::value_ptr(projectionMatrix));

			TransformComponent* TransformComponent = inTransforms.get(meshRenderer->entityID);
			shader->setMat4("model", glm::value_ptr(TransformComponent->model));

			glBindVertexArray(meshRenderer->VAOID);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
	}

	void swap(GLFWwindow* inWindow)
	{
		glfwSwapBuffers(inWindow);
	}

	GLuint addShader(const char* inVertexPath, const char* inFragmentPath)
	{
		Shader shader;
		shader.load(inVertexPath, inFragmentPath);

		shaders.push_back(shader);
		return shader.ID;
	}

	GLuint createVAO(const Mesh& inMesh)
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
		glVertexAttribPointer(vertexAttribPointer, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(vertexAttribPointer);

		vertexAttribPointer = 1; // texcoord layout location in vertex shader
		glVertexAttribPointer(vertexAttribPointer, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(vertexAttribPointer);

		glBindVertexArray(0);

		return VAO;
	}

private:
	Shader* getShader(GLuint inShaderID)
	{
		for (auto& shader : shaders)
		{
			if (shader.ID == inShaderID)
			{
				return &shader;
			}
		}

		return nullptr;
	}

private:
	std::vector<Shader> shaders;
};

