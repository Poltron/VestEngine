#pragma once

#include <vector>

#include "Render/GPUHandle.h"

class Mesh;

class VulkanMeshProxy
{
	uint32_t vertexCount;
	GPUAllocHandle vertexBuffer;
	uint32_t indexCount;
	GPUAllocHandle indexBuffer;

public:
	VulkanMeshProxy() = delete;
	VulkanMeshProxy(const Mesh* inMesh);
	~VulkanMeshProxy();
	
	VulkanMeshProxy(const VulkanMeshProxy& inOther) = delete;
	VulkanMeshProxy& operator=(const VulkanMeshProxy& inOther) = delete;

	VulkanMeshProxy(VulkanMeshProxy&& inOther) noexcept;
	VulkanMeshProxy& operator=(VulkanMeshProxy&& inOther) noexcept;

	GPUAllocHandle getVertexBuffer() const { return vertexBuffer; }
	GPUAllocHandle getIndexBuffer() const { return indexBuffer; }

	uint32_t getVertexCount() const { return vertexCount; }
	uint32_t getIndexCount() const { return indexCount; }
};