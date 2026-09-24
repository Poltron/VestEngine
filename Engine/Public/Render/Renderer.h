#pragma once

#include "Render/GraphicResourceHandle.h"
#include "Render/ShaderParameterCollection.h"
#include "Core/Ensure.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

class Camera;
class Scene;

struct FrameInfo
{
	unsigned int renderedMeshTotal;
	unsigned int culledMeshTotal;

	void reset()
	{
		renderedMeshTotal = 0;
		culledMeshTotal = 0;
	}
};

class Renderer
{
public:
	void clear();
	void render(Scene& inScene, double inCurrentFrame);
	void swap();
	void setActiveCamera(Camera* inCamera);
	Camera& getActiveCamera();

private:
	Camera* activeCamera;
//
public:
	const FrameInfo& getFrameInfo() const { return frameInfo; }

private:
	FrameInfo frameInfo;

//
public:
	void updateLightParameters(Scene& inScene);

	void loadDefaultShaders();

	ResourceHandle getLitShader() { return litShaderHandle; }
	void setLitShader(ResourceHandle inResourceHandle) { litShaderHandle = inResourceHandle; }
	ResourceHandle getUnlitShader() { return unlitShaderHandle; }
	void setUnlitShader(ResourceHandle inResourceHandle) { unlitShaderHandle = inResourceHandle; }
	ResourceHandle getSolidColorShader() { return solidColorShaderHandle; }
	void setSolidColorShader(ResourceHandle inResourceHandle) { solidColorShaderHandle = inResourceHandle; }

private:
	ShaderParameterCollection globalShaderParameters;

	ResourceHandle solidColorShaderHandle;
	ResourceHandle litShaderHandle;
	ResourceHandle unlitShaderHandle;
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}