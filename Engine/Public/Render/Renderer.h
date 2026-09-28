#pragma once

#include <vector>

#include "ECS/ComponentManager.h"
#include "ECS/Entity.h"
#include "ECS/Systems/EntityDebugShapesSystem.h"
#include "Render/DebugShapes.h"
#include "Render/GraphicResourceHandle.h"
#include "Render/ShaderParameterCollection.h"
#include "Core/Ensure.h"
#include "Core/ResourceHandle.h"

#define MAX_POINT_LIGHTS 3

class Camera;
class Model;
class Scene;

struct FrameInfo
{
	size_t renderedMeshTotal;
	size_t culledMeshTotal;
	size_t debugShapesTotal;

	size_t shaderPrograms;
	size_t drawCalls;

	void reset()
	{
		renderedMeshTotal = 0;
		culledMeshTotal = 0;
		debugShapesTotal = 0;
		shaderPrograms = 0;
		drawCalls = 0;
	}
};

class Renderer
{
public:
	void clear();
	void render(Scene& inScene);
	void swap();

private:
	void renderMainPass(Scene& inScene);
	void renderDebugPass(Scene& inScene);

//
public:
	void setActiveCamera(Camera* inCamera);
	Camera& getActiveCamera();

private:
	Camera* activeCamera;

//
public:
	enum class EPrimitiveMode
	{
		POINTS,
		LINES,
		TRIANGLES
	};

private:
	Shader* useShaderProgram(ResourceHandle inShaderHandle);

	void drawModel(const Model& inModel);
	void drawArrays(GraphicResourceHandle inVAO, EPrimitiveMode inPrimitiveType, int inOffset, size_t inCount);
	void drawArraysInstanced(GraphicResourceHandle inVAO, EPrimitiveMode inPrimitiveType, int inOffset, size_t inSize, size_t inCount);
	void drawElements(GraphicResourceHandle inVAO, EPrimitiveMode inPrimiveType, size_t inCount);

private:
	struct RendererState
	{
		GraphicResourceHandle shaderProgram = UINT32_MAX;
	};
	RendererState state;

//
public:
	void loadDebugShapes();
	void updateDebugShapes(Scene& inScene);

	void addPoint(const glm::vec3& inPosition, float inSize, const glm::vec3& inColor);
	void addLine(const glm::vec3& inStart, const glm::vec3& inEnd, float inSize, const glm::vec3& inColor);
	void addSphere(const glm::vec3& inPosition, const glm::vec3& inRotation, float inRadius, const glm::vec3& inColor);
	void addBox(const glm::vec3& inPosition, const glm::vec3& inRotation, const glm::vec3& inScale, const glm::vec3& inColor);

	void addMovableSphere(Entity inEntity, const glm::vec3& inColor);
	void removeMovableSphere(Entity inEntity);

private:
	std::vector<DebugShapePrimitive> debugShapePrimitives;
	std::vector<DebugShapePrimitiveInstance> pointDebugShapeInstances;
	std::vector<DebugShapePrimitiveInstance> lineDebugShapeInstances;

	std::vector<DebugShapeMesh> debugShapeMeshes;
	std::vector<DebugShapeMeshInstance> boxDebugShapeInstances;
	std::vector<DebugShapeMeshInstance> sphereDebugShapeInstances;

	EntityDebugShapeMeshInstances entitySphereInstances;

	// todo: let this system's file in the renderer module but "register" it to the ECS
	EntityDebugShapesSystem updateEntityDebugShapesSystem;

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

	ResourceHandle getDebugPrimitiveShader() { return debugPrimitiveShaderHandle; }
	void setDebugPrimitiveShader(ResourceHandle inResourceHandle) { debugPrimitiveShaderHandle = inResourceHandle; }
	ResourceHandle getDebugMeshShader() { return debugMeshShaderHandle; }
	void setDebugMeshShader(ResourceHandle inResourceHandle) { debugMeshShaderHandle = inResourceHandle; }

private:
	ShaderParameterCollection globalShaderParameters;

	ResourceHandle litShaderHandle;
	ResourceHandle unlitShaderHandle;
	ResourceHandle solidColorShaderHandle;

	ResourceHandle debugPrimitiveShaderHandle;
	ResourceHandle debugMeshShaderHandle;

//
public:
	const FrameInfo& getFrameInfo() const { return frameInfo; }

	FrameInfo frameInfo;
};

namespace render
{
	bool initialize();
	void shutdown();
	Renderer* getRenderer();
}