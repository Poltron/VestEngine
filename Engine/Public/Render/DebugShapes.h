#pragma once

#include "glm/glm.hpp"

#include "ECS/Entity.h"
#include "Render/GraphicResourceHandle.h"

struct DebugShapePrimitiveInstance
{
	glm::vec3 position;
	glm::vec3 color;
	float size;
};

class DebugShapePrimitive
{
	GraphicResourceHandle VAO;
	GraphicResourceHandle VBO;

	size_t instanceCount;

public:
	DebugShapePrimitive();
	~DebugShapePrimitive();

	DebugShapePrimitive(const DebugShapePrimitive& inOther) = delete;
	DebugShapePrimitive& operator=(const DebugShapePrimitive& inOther) = delete;

	DebugShapePrimitive(DebugShapePrimitive&& inOther) noexcept;
	DebugShapePrimitive& operator=(DebugShapePrimitive&& inOther) noexcept;

	GraphicResourceHandle getVAO() const { return VAO; }
	GraphicResourceHandle getVBO() const { return VBO; }

	void updateVBO(std::vector<DebugShapePrimitiveInstance>& inInstances);

private:
	void setupMesh();
};


//
struct DebugShapeMeshInstance
{
	glm::vec3 color;
	glm::mat4 model;
};

struct EntityDebugShapeMeshInstances
{
	std::vector<Entity> entities;
	std::vector<DebugShapeMeshInstance> instances;
};

class DebugShapeMesh
{
	std::vector<glm::vec3> vertices;

	GraphicResourceHandle VAO;
	GraphicResourceHandle VBOmesh;
	GraphicResourceHandle VBOinstances;

	size_t instanceCount;

public:
	DebugShapeMesh() = delete;
	DebugShapeMesh(std::vector<glm::vec3>&& inVertices);
	~DebugShapeMesh();

	DebugShapeMesh(const DebugShapeMesh& inOther) = delete;
	DebugShapeMesh& operator=(const DebugShapeMesh& inOther) = delete;

	DebugShapeMesh(DebugShapeMesh&& inOther) noexcept;
	DebugShapeMesh& operator=(DebugShapeMesh&& inOther) noexcept;

	GraphicResourceHandle getVAO() const { return VAO; }
	GraphicResourceHandle getVBOMesh() const { return VBOmesh; }
	GraphicResourceHandle getVBOInstances() const { return VBOinstances; }
	size_t getInstanceCount() const { return instanceCount; }
	size_t getSize() const { return instanceCount * sizeof(DebugShapeMeshInstance); }

	void updateInstances(const std::vector<DebugShapeMeshInstance>& inInstances);

private:
	void setupMesh();
};

