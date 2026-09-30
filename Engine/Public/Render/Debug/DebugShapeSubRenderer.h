#pragma once

#include "Core/ResourceHandle.h"
#include "ECS/Systems/EntityDebugShapesSystem.h"
#include "Render/Debug/DebugShapes.h"
#include "Render/FrameInfo.h"

class Scene;
class Renderer;

enum class EDebugShapePrimitive : unsigned int
{
	POINT = 0,
	LINE,
	ENUM_SIZE
};

enum class EDebugShapeMesh : unsigned int
{
	BOX = 0,
	SPHERE,
	ENUM_SIZE
};

class DebugShapeSubRenderer
{
	ResourceHandle debugPrimitiveShaderHandle;
	ResourceHandle debugMeshShaderHandle;

	std::vector<DebugShapePrimitive> debugShapePrimitives;
	std::vector<DebugShapePrimitiveInstance> pointDebugShapeInstances;
	std::vector<DebugShapePrimitiveInstance> lineDebugShapeInstances;

	std::vector<DebugShapeMesh> debugShapeMeshes;
	std::vector<DebugShapeMeshInstance> boxDebugShapeInstances;
	std::vector<DebugShapeMeshInstance> sphereDebugShapeInstances;

	EntityDebugShapeMeshInstances entitySphereInstances;

	// todo: let this system's file in the renderer module but "register" it to the ECS
	EntityDebugShapesSystem updateEntityDebugShapesSystem;

public:
	void loadShaders();
	void loadDebugShapes();
	void render(Renderer& inRenderer, FrameInfo& inFrameInfo);
	void updateDebugShapes(Scene& inScene);

	ResourceHandle getDebugPrimitiveShader() { return debugPrimitiveShaderHandle; }
	void setDebugPrimitiveShader(ResourceHandle inResourceHandle) { debugPrimitiveShaderHandle = inResourceHandle; }
	ResourceHandle getDebugMeshShader() { return debugMeshShaderHandle; }
	void setDebugMeshShader(ResourceHandle inResourceHandle) { debugMeshShaderHandle = inResourceHandle; }

	void addPoint(const glm::vec3& inPosition, float inSize, const glm::vec3& inColor);
	void addLine(const glm::vec3& inStart, const glm::vec3& inEnd, float inSize, const glm::vec3& inColor);
	void addSphere(const glm::vec3& inPosition, const glm::vec3& inRotation, float inRadius, const glm::vec3& inColor);
	void addBox(const glm::vec3& inPosition, const glm::vec3& inRotation, const glm::vec3& inScale, const glm::vec3& inColor);

	void addMovableSphere(Entity inEntity, const glm::vec3& inColor);
	void removeMovableSphere(Entity inEntity);
};