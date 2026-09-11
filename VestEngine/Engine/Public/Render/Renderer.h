#pragma once

#include "Render/ShaderParameterCollection.h"
#include "Core/Ensure.h"

#define MAX_POINT_LIGHTS 3

template<typename T>
class ComponentManager;
class ResourcesManager;

class Camera;
class Mesh;
class Shader;

struct DirectionalLightComponent;
struct MeshRendererComponent;
struct PointLightComponent;
struct LocalTransformComponent;
struct WorldTransformComponent;

class Renderer
{
public:
	void clear();
	void render(ResourcesManager& inResourcesManager
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, double inCurrentFrame);
	void swap();

	//
	void fillLightParameters(const ComponentManager<WorldTransformComponent>& inTransforms
		, const ComponentManager<PointLightComponent>& inPointLights
		, const ComponentManager<DirectionalLightComponent>& inDirectionalLights);
	void setActiveCamera(Camera* inCamera);

	// note : currently unused since setupMesh does this inside mesh, should be used again though, renderer should handle OpenGL specifics
	//GLuint createVAO(const Mesh& inMesh);
	
private:
	ShaderParameterCollection globalShaderParameters;
	Camera* activeCamera;
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}