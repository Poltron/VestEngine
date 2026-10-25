#pragma once

#include <vector>

#include "Render/Debug/DebugShapeSubRenderer.h"
#include "Render/FrameInfo.h"
#include "Render/GPUHandle.h"
#include "Render/ShaderParameterCollection.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

class Camera;
class Model;
class Scene;

class Renderer
{
	ShaderParameterCollection globalShaderParameters;

	Camera* activeCamera;

	GPUHandle litShaderHandle;
	GPUHandle unlitShaderHandle;
	GPUHandle solidColorShaderHandle;

protected:
	DebugShapeSubRenderer debugShapeSubRenderer;
	FrameInfo frameInfo;

public:
	void clear();
	void render(Scene& inScene);
	void swap();

	void updateDebugShapes(Scene& inScene);

private:
	void renderMainPass(Scene& inScene);
	void renderDebugPass(Scene& inScene);

//
public:
	void drawModel(const Model& inModel);

//
public:
	void loadDefaultShaders();
	void updateLightParameters(Scene& inScene);

	GPUHandle getLitShader() { return litShaderHandle; }
	void setLitShader(GPUHandle inResourceHandle) { litShaderHandle = inResourceHandle; }
	GPUHandle getUnlitShader() { return unlitShaderHandle; }
	void setUnlitShader(GPUHandle inResourceHandle) { unlitShaderHandle = inResourceHandle; }
	GPUHandle getSolidColorShader() { return solidColorShaderHandle; }
	void setSolidColorShader(GPUHandle inResourceHandle) { solidColorShaderHandle = inResourceHandle; }

	void setActiveCamera(Camera* inCamera);
	Camera& getActiveCamera();

	DebugShapeSubRenderer& getDebugShapeSubRenderer() { return debugShapeSubRenderer; }
	const FrameInfo& getFrameInfo() const { return frameInfo; }
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}