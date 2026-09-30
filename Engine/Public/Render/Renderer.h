#pragma once

#include <vector>

#include "ECS/ComponentManager.h"
#include "ECS/Entity.h"
#include "Render/Debug/DebugShapeSubRenderer.h"
#include "Render/GraphicResourceHandle.h"
#include "Render/FrameInfo.h"
#include "Render/ShaderParameterCollection.h"
#include "Core/Ensure.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

class Camera;
class Model;
class Scene;

enum class EPrimitiveMode
{
	POINTS,
	LINES,
	TRIANGLES
};

class Renderer
{
	struct RendererState
	{
		GraphicResourceHandle shaderProgram = UINT32_MAX;
	};
	RendererState state;

	ShaderParameterCollection globalShaderParameters;

	Camera* activeCamera;

	ResourceHandle litShaderHandle;
	ResourceHandle unlitShaderHandle;
	ResourceHandle solidColorShaderHandle;

	FrameInfo frameInfo;

protected:
	DebugShapeSubRenderer debugShapeSubRenderer;

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
	Shader* useShaderProgram(ResourceHandle inShaderHandle);

	void drawModel(const Model& inModel);
	void drawArrays(GraphicResourceHandle inVAO, EPrimitiveMode inPrimitiveType, int inOffset, size_t inCount);
	void drawArraysInstanced(GraphicResourceHandle inVAO, EPrimitiveMode inPrimitiveType, int inOffset, size_t inSize, size_t inCount);
	void drawElements(GraphicResourceHandle inVAO, EPrimitiveMode inPrimiveType, size_t inCount);

//
public:
	void loadDefaultShaders();
	void updateLightParameters(Scene& inScene);

	ResourceHandle getLitShader() { return litShaderHandle; }
	void setLitShader(ResourceHandle inResourceHandle) { litShaderHandle = inResourceHandle; }
	ResourceHandle getUnlitShader() { return unlitShaderHandle; }
	void setUnlitShader(ResourceHandle inResourceHandle) { unlitShaderHandle = inResourceHandle; }
	ResourceHandle getSolidColorShader() { return solidColorShaderHandle; }
	void setSolidColorShader(ResourceHandle inResourceHandle) { solidColorShaderHandle = inResourceHandle; }

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