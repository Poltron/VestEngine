#pragma once

#include <vector>

#include "Render/Debug/DebugShapeSubRenderer.h"
#include "Render/FrameInfo.h"
#include "Render/GPUHandle.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

class Camera;
class Model;
class Scene;

class Renderer
{
	Camera* activeCamera;

	ResourceHandle litMaterialHandle;
	ResourceHandle unlitMaterialHandle;
	ResourceHandle solidColorMaterialHandle;

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
	void loadDefaultMaterials();
	void updateLightParameters(Scene& inScene);

	ResourceHandle getLitMaterial() { return litMaterialHandle; }
	void setLitMaterial(ResourceHandle inResourceHandle) { litMaterialHandle = inResourceHandle; }
	ResourceHandle getUnlitMaterial() { return unlitMaterialHandle; }
	void setUnlitMaterial(ResourceHandle inResourceHandle) { unlitMaterialHandle = inResourceHandle; }
	ResourceHandle getSolidColorMaterial() { return solidColorMaterialHandle; }
	void setSolidColorMaterial(ResourceHandle inResourceHandle) { solidColorMaterialHandle = inResourceHandle; }

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