#include "Render/Debug/DebugShapes.h"

#include <iostream>

#include "Render/GraphicResourceHandle.h"
#include "RenderBackend/RenderBackend.h"

DebugShapePrimitive::DebugShapePrimitive(size_t inCapacity)
	: capacity(inCapacity), instanceCount(0)
{
	VAO = {};
	VBO = {};

	initialize();
}

DebugShapePrimitive::~DebugShapePrimitive()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, VBO);
}

DebugShapePrimitive::DebugShapePrimitive(DebugShapePrimitive&& inOther) noexcept
	: VAO(inOther.VAO), VBO(inOther.VBO), capacity(inOther.capacity), instanceCount(inOther.instanceCount)
{
	inOther.VAO = {};
	inOther.VBO = {};
	inOther.capacity = 0;
	inOther.instanceCount = 0;
}

DebugShapePrimitive& DebugShapePrimitive::operator=(DebugShapePrimitive&& inOther) noexcept
{
	if (this != &inOther)
	{
		VAO = inOther.VAO;
		VBO = inOther.VBO;
		capacity = inOther.capacity;
		instanceCount = inOther.instanceCount;

		inOther.VAO = {};
		inOther.VBO = {};
		inOther.capacity = 0;
		inOther.instanceCount = 0;
	}
	return *this;
}

void DebugShapePrimitive::initialize()
{
	VAO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTEX_ARRAY);
	VBO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTICES);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);

	reallocateVBO(capacity);

	const unsigned int divisor = 0;
	render::getRenderBackend()->setVertexLayoutData(0, 3, sizeof(DebugShapePrimitiveInstance), (void*)offsetof(DebugShapePrimitiveInstance, position), divisor);
	render::getRenderBackend()->setVertexLayoutData(1, 3, sizeof(DebugShapePrimitiveInstance), (void*)offsetof(DebugShapePrimitiveInstance, color), divisor);
	render::getRenderBackend()->setVertexLayoutData(2, 1, sizeof(DebugShapePrimitiveInstance), (void*)offsetof(DebugShapePrimitiveInstance, size), divisor);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
}

void DebugShapePrimitive::reallocateVBO(size_t inNewCapacity)
{
	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBO);
	render::getRenderBackend()->allocateGPUBuffer(EBufferType::VERTICES, inNewCapacity * sizeof(DebugShapePrimitiveInstance), nullptr, EBufferUsage::DYNAMIC);

	capacity = inNewCapacity;
}

void DebugShapePrimitive::resetInstanceCount()
{
	instanceCount = 0;
}

void DebugShapePrimitive::updateVBO(std::vector<DebugShapePrimitiveInstance>& inInstances)
{
	const size_t newInstanceCount = instanceCount + inInstances.size();
	if (newInstanceCount > capacity)
	{
		size_t newCapacity = std::max(newInstanceCount, instanceCount * 2);
		reallocateVBO(newCapacity);
	}
	else
	{
		render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBO);
	}

	render::getRenderBackend()->setGPUBufferData(EBufferType::VERTICES, 0, inInstances.size() * sizeof(DebugShapePrimitiveInstance), inInstances.data());

	instanceCount = newInstanceCount;
}

//
DebugShapeMesh::DebugShapeMesh(std::vector<glm::vec3>&& inVertices, size_t inCapacity)
	: vertices(inVertices), capacity(inCapacity), instanceCount(0)
{
	VAO = {};
	VBOmesh = {};
	VBOinstances = {};

	initialize();
}

DebugShapeMesh::~DebugShapeMesh()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, VBOmesh);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, VBOinstances);
}

DebugShapeMesh::DebugShapeMesh(DebugShapeMesh&& inOther) noexcept
	: VAO(inOther.VAO), VBOmesh(inOther.VBOmesh), VBOinstances(inOther.VBOinstances), capacity(inOther.capacity), instanceCount(inOther.instanceCount)
{
	vertices.clear();
	vertices = std::move(inOther.vertices);

	inOther.VAO = {};
	inOther.VBOmesh = {};
	inOther.VBOinstances = {};
}

DebugShapeMesh& DebugShapeMesh::operator=(DebugShapeMesh&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertices.clear();
		vertices = std::move(inOther.vertices);

		VAO = inOther.VAO;
		VBOmesh = inOther.VBOmesh;
		VBOinstances = inOther.VBOinstances;
		capacity = inOther.capacity;
		instanceCount = inOther.instanceCount;

		inOther.VAO = {};
		inOther.VBOmesh = {};
		inOther.VBOinstances = {};
		inOther.capacity = capacity;
		inOther.instanceCount = instanceCount;
	}
	return *this;
}

void DebugShapeMesh::initialize()
{
	VAO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTEX_ARRAY);
	VBOmesh = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTICES);
	VBOinstances = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTICES);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);

	// vertices
	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBOmesh);
	size_t size = vertices.size() * sizeof(glm::vec3);
	render::getRenderBackend()->allocateGPUBuffer(EBufferType::VERTICES, size, vertices.data(), EBufferUsage::STATIC);

	render::getRenderBackend()->setVertexLayoutData(0, 3, sizeof(glm::vec3), (void*)0, 0);

	// instances
	reallocateVBOinstances(capacity);

	const unsigned int divisor = 1;
	render::getRenderBackend()->setVertexLayoutData(1, 3, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, color)), divisor);
	render::getRenderBackend()->setVertexLayoutData(2, 4, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model)), divisor);
	render::getRenderBackend()->setVertexLayoutData(3, 4, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 1), divisor);
	render::getRenderBackend()->setVertexLayoutData(4, 4, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 2), divisor);
	render::getRenderBackend()->setVertexLayoutData(5, 4, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 3), divisor);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
}

void DebugShapeMesh::reallocateVBOinstances(size_t inNewCapacity)
{
	const size_t newSize = inNewCapacity * sizeof(DebugShapeMeshInstance);
	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBOinstances);
	render::getRenderBackend()->allocateGPUBuffer(EBufferType::VERTICES, newSize, nullptr, EBufferUsage::DYNAMIC);

	capacity = inNewCapacity;
}

void DebugShapeMesh::addInstances(const std::vector<DebugShapeMeshInstance>& inInstances)
{
	const size_t newInstanceCount = instanceCount + inInstances.size();
	if (newInstanceCount > capacity)
	{
		size_t newCapacity = std::max(newInstanceCount, instanceCount * 2);
		reallocateVBOinstances(newCapacity);
	}
	else
	{
		render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBOinstances);
	}

	const size_t offset = instanceCount * sizeof(DebugShapeMeshInstance);
	const size_t size = inInstances.size() * sizeof(DebugShapeMeshInstance);
	render::getRenderBackend()->setGPUBufferData(EBufferType::VERTICES, offset, size, (void*)inInstances.data());

	instanceCount = newInstanceCount;
}