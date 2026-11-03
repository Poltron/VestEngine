#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

class ResourcesManager;

class VulkanMesh
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<ResourceHandle> textures;

	GPUHandle vertexBuffer;
	GPUHandle indexBuffer;

public:
	VulkanMesh() = delete;
	VulkanMesh(std::vector<Vertex>&& inVertices, std::vector<uint16_t>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~VulkanMesh();
	
	VulkanMesh(const VulkanMesh& inOther) = delete;
	VulkanMesh& operator=(const VulkanMesh& inOther) = delete;

	VulkanMesh(VulkanMesh&& inOther) noexcept;
	VulkanMesh& operator=(VulkanMesh&& inOther) noexcept;

	GPUHandle getVertexBuffer() const { return vertexBuffer; }
	GPUHandle getIndexBuffer() const { return indexBuffer; }

	size_t getVerticesSize() const { return vertices.size(); }
	size_t getIndicesSize() const { return indices.size(); }

	const std::vector<ResourceHandle>& getTextures() const { return textures; }

private:
	void setupMesh();
};