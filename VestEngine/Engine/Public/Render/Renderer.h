#pragma once

#include "Render/GraphicResourceHandle.h"
#include "Render/ShaderParameterCollection.h"
#include "Core/Ensure.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

template<typename T>
class ComponentManager;
class ResourcesManager;

class Camera;
class Mesh;
class Scene;
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
	void render(Scene& inScene, double inCurrentFrame);
	void swap();
	void setActiveCamera(Camera* inCamera);
	
private:
	Camera* activeCamera;

//
public:
	void updateLightParameters(Scene& inScene);

	void loadDefaultShaders();

	ResourceHandle getLitShader() { return litShaderHandle; }
	void setLitShader(ResourceHandle inResourceHandle) { litShaderHandle = inResourceHandle; }
	ResourceHandle getUnlitShader() { return unlitShaderHandle; }
	void setUnlitShader(ResourceHandle inResourceHandle) { unlitShaderHandle = inResourceHandle; }
	ResourceHandle getOutlineShader() { return outlineShaderHandle; }
	void setOutlineShader(ResourceHandle inResourceHandle) { outlineShaderHandle = inResourceHandle; }

private:
	ShaderParameterCollection globalShaderParameters;

	ResourceHandle outlineShaderHandle;
	ResourceHandle litShaderHandle;
	ResourceHandle unlitShaderHandle;
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}