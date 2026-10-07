#include "Render/Debug/DebugShapes.h"

#include <iostream>
#include "glad/glad.h"

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

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, position)));

	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, color)));

	glEnableVertexAttribArray(2); // color
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(DebugShapePrimitiveInstance), (void*)(offsetof(DebugShapePrimitiveInstance, size)));

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);

	while (GLenum err = glGetError())
	{
		if (err != GL_NO_ERROR)
		{
			std::cerr << "DebugShapePrimitive::initialize failed: " << err << std::endl;
		}
	}
}

void DebugShapePrimitive::reallocateVBO(size_t inNewCapacity)
{
	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBO);
	glBufferData(GL_ARRAY_BUFFER, inNewCapacity * sizeof(DebugShapePrimitiveInstance), nullptr, GL_DYNAMIC_DRAW);
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

	glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(inInstances.size() * sizeof(DebugShapePrimitiveInstance)), inInstances.data());

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

	glEnableVertexAttribArray(0); // vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glVertexAttribDivisor(0, 0);

	// instances
	reallocateVBOinstances(capacity);

	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, color)));

	glEnableVertexAttribArray(2); // model[0]
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model)));
	glEnableVertexAttribArray(3); // model[1]
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 1));
	glEnableVertexAttribArray(4); // model[2]
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 2));
	glEnableVertexAttribArray(5); // model[3]
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShapeMeshInstance), (void*)(offsetof(DebugShapeMeshInstance, model) + sizeof(glm::vec4) * 3));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

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