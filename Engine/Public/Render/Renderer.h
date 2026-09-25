#pragma once

#include <vector>

#include "Render/PrimitiveMesh.h"
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

	unsigned int renderedPrimitivesTotal;
	unsigned int culledPrimitivesTotal;

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
	void render(Scene& inScene);
	void swap();

private:
	void renderMeshRenderersComponents(Scene& inScene);

//
public:
	void setActiveCamera(Camera* inCamera);
	Camera& getActiveCamera();

private:
	Camera* activeCamera;

//
public:
	void loadPrimitiveMeshes();

	void addPoint(const glm::vec3& inPosition, unsigned int inSize, const glm::vec3& inColor);
	void addLine(const glm::vec3& inStart, const glm::vec3& inEnd, const glm::vec3& inColor);
	void addSphere(const glm::vec3& inPosition, const glm::vec3& inRotation, float inRadius, const glm::vec3& inColor);
	void addBox(const glm::vec3& inPosition, const glm::vec3& inRotation, const glm::vec3& inScale, const glm::vec3& inColor);

private:
	void renderPrimitives(Scene& inScene);

	std::vector<PointPrimitiveInstance> pointPrimitiveInstances;
	std::vector<LinePrimitiveInstance> linePrimitiveInstances;
	std::vector<BoxPrimitiveInstance> boxPrimitiveInstances;
	std::vector<SpherePrimitiveInstance> spherePrimitiveInstances;

	std::vector<PrimitiveMesh> primitiveMeshes;

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
	ResourceHandle getPrimitiveShader() { return primitiveShaderHandle; }
	void setPrimitiveShader(ResourceHandle inResourceHandle) { primitiveShaderHandle = inResourceHandle; }

private:
	ShaderParameterCollection globalShaderParameters;

	ResourceHandle primitiveShaderHandle;
	ResourceHandle solidColorShaderHandle;
	ResourceHandle litShaderHandle;
	ResourceHandle unlitShaderHandle;

//
public:
	const FrameInfo& getFrameInfo() const { return frameInfo; }

private:
	FrameInfo frameInfo;
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}