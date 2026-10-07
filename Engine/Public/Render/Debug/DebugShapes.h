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
	GPUHandle VAO;
	GPUHandle VBO;

	size_t capacity;
	size_t instanceCount;

public:
	DebugShapePrimitive() = delete;
	DebugShapePrimitive(size_t inCapacity);
	~DebugShapePrimitive();

	DebugShapePrimitive(const DebugShapePrimitive& inOther) = delete;
	DebugShapePrimitive& operator=(const DebugShapePrimitive& inOther) = delete;

	DebugShapePrimitive(DebugShapePrimitive&& inOther) noexcept;
	DebugShapePrimitive& operator=(DebugShapePrimitive&& inOther) noexcept;

	GPUHandle getVAO() const { return VAO; }
	GPUHandle getVBO() const { return VBO; }

	void resetInstanceCount();
	void updateVBO(std::vector<DebugShapePrimitiveInstance>& inInstances);

private:
	void initialize();
	void reallocateVBO(size_t inNewCapacity);
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

	GPUHandle VAO;
	GPUHandle VBOmesh;
	GPUHandle VBOinstances;

	size_t capacity;
	size_t instanceCount;

public:
	DebugShapeMesh() = delete;
	DebugShapeMesh(std::vector<glm::vec3>&& inVertices, size_t inCapacity);
	~DebugShapeMesh();

	DebugShapeMesh(const DebugShapeMesh& inOther) = delete;
	DebugShapeMesh& operator=(const DebugShapeMesh& inOther) = delete;

	DebugShapeMesh(DebugShapeMesh&& inOther) noexcept;
	DebugShapeMesh& operator=(DebugShapeMesh&& inOther) noexcept;

	GPUHandle getVAO() const { return VAO; }
	GPUHandle getVBOMesh() const { return VBOmesh; }
	GPUHandle getVBOInstances() const { return VBOinstances; }
	size_t getInstanceCount() const { return instanceCount; }
	size_t getSize() const { return instanceCount * sizeof(DebugShapeMeshInstance); }

	void resetInstances() { instanceCount = 0; }
	void addInstances(const std::vector<DebugShapeMeshInstance>& inInstances);

private:
	void initialize();
	void reallocateVBOinstances(size_t inNewCapacity);
};

