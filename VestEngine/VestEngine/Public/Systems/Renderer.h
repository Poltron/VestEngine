#pragma once
#include <vector>
#include "Resources/Shader.h"

#include "Systems/ShaderParameterCollection.h"

#define MAX_POINT_LIGHTS 3

template<typename T>
class ComponentManager;
class ResourcesManager;

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
	void initialize();

	void clear();
	void render(ResourcesManager& inResourcesManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, double inCurrentFrame);
	void swap(GLFWwindow* inWindow);

	//
	void fillLightParameters(const ComponentManager<TransformComponent>& inTransforms
		, const ComponentManager<PointLightComponent>& inPointLights
		, const ComponentManager<DirectionalLightComponent>& inDirectionalLights);
	void setActiveCamera(Camera* inCamera);

	// note @ech: currently unused since setupMesh does this inside mesh, should be used again though, renderer should handle OpenGL specifics
	//GLuint createVAO(const Mesh& inMesh);

public:
	ShaderParameterCollection globalShaderParameters;

private:
	Camera* activeCamera;
};

