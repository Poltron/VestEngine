#include "RenderBackend_Vulkan/VulkanMeshProxy.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "Core/Resources/Mesh.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"

VulkanMeshProxy::VulkanMeshProxy(const Mesh* inMesh)
	: vertexBuffer(GPUAllocHandle::invalid), indexBuffer(GPUAllocHandle::invalid)
{
	VulkanDevice& VulkanDevice = render::getRenderBackend()->getDevice();

	vertexCount = sizeof(Vertex) * inMesh->getVertexCount();
	vertexBuffer = VulkanDevice.createAndFillBuffer(vertexCount, (void*)inMesh->getVertices().data(), EBufferType::VERTICES);

	indexCount = sizeof(uint16_t) * inMesh->getIndexCount();
	indexBuffer = VulkanDevice.createAndFillBuffer(indexCount, (void*)inMesh->getIndices().data(), EBufferType::INDICES);
}

VulkanMeshProxy::~VulkanMeshProxy()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, vertexBuffer);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, indexBuffer);
}

VulkanMeshProxy::VulkanMeshProxy(VulkanMeshProxy&& inOther) noexcept
	: vertexBuffer(inOther.vertexBuffer), vertexCount(0), indexBuffer(inOther.indexBuffer), indexCount(0)
{
	inOther.vertexBuffer = GPUAllocHandle::invalid;
	inOther.vertexCount = 0;
	inOther.indexBuffer = GPUAllocHandle::invalid;
	inOther.indexCount = 0;
}

VulkanMeshProxy& VulkanMeshProxy::operator=(VulkanMeshProxy&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertexBuffer = inOther.vertexBuffer;
		vertexCount = inOther.vertexCount;;
		indexBuffer = inOther.indexBuffer;
		indexCount = inOther.indexCount;

		inOther.vertexBuffer = GPUAllocHandle::invalid;
		inOther.vertexCount = 0;
		inOther.indexBuffer = GPUAllocHandle::invalid;
		inOther.indexCount = 0;
	}
	return *this;
}