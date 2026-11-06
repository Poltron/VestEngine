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

	vertexCount = inMesh->getVertexCount();
	vertexBuffer = VulkanDevice.createAndFillBuffer(sizeof(Vertex) * vertexCount, (void*)inMesh->getVertices().data(), EBufferType::VERTICES);

	indexCount = inMesh->getIndexCount();
	indexBuffer = VulkanDevice.createAndFillBuffer(sizeof(uint16_t) * indexCount, (void*)inMesh->getIndices().data(), EBufferType::INDICES);
}

VulkanMeshProxy::~VulkanMeshProxy()
{
	if (vertexBuffer.isValid())
	{
		render::getRenderBackend()->getDevice().destroyBuffer(vertexBuffer);
	}
	if (indexBuffer.isValid())
	{
		render::getRenderBackend()->getDevice().destroyBuffer(indexBuffer);
	}
}

VulkanMeshProxy::VulkanMeshProxy(VulkanMeshProxy&& inOther) noexcept
	: vertexBuffer(inOther.vertexBuffer), vertexCount(inOther.vertexCount), indexBuffer(inOther.indexBuffer), indexCount(inOther.indexCount)
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
		vertexCount = inOther.vertexCount;
		indexBuffer = inOther.indexBuffer;
		indexCount = inOther.indexCount;

		inOther.vertexBuffer = GPUAllocHandle::invalid;
		inOther.vertexCount = 0;
		inOther.indexBuffer = GPUAllocHandle::invalid;
		inOther.indexCount = 0;
	}
	return *this;
}