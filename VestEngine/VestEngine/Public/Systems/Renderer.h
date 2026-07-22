#pragma once
#include <vector>
#include "Resources/Shader.h"

template<typename T>
class ComponentManager;

class Camera;
class Mesh;
class Shader;

struct GLFWwindow;

struct DirectionalLightComponent;
struct MeshRendererComponent;
struct PointLightComponent;
struct TransformComponent;

class Renderer
{
public:
	void clear();
	void render(ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<PointLightComponent>& inPointLights
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, double inCurrentFrame);
	void swap(GLFWwindow* inWindow);

	void setActiveCamera(Camera* inCamera);

	GLuint addShader(const char* inVertexPath, const char* inFragmentPath);
	GLuint createVAO(const Mesh& inMesh);

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
	Camera* activeCamera;
};

