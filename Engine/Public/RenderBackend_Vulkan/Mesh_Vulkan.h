#pragma once

#include <vector>

#include "Core/ResourceHandle.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

class ResourcesManager;

class Mesh_Vulkan
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures;

	GPUHandle vertexBuffer;
	GPUHandle indexBuffer;

public:
	Mesh_Vulkan() = delete;
	Mesh_Vulkan(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~Mesh_Vulkan();
	
	Mesh_Vulkan(const Mesh_Vulkan& inOther) = delete;
	Mesh_Vulkan& operator=(const Mesh_Vulkan& inOther) = delete;

	Mesh_Vulkan(Mesh_Vulkan&& inOther) noexcept;
	Mesh_Vulkan& operator=(Mesh_Vulkan&& inOther) noexcept;

	GPUHandle getVertexBuffer() const { return vertexBuffer; }
	GPUHandle getIndexBuffer() const { return indexBuffer; }

	size_t getVerticesSize() const { return vertices.size(); }
	size_t getIndicesSize() const { return indices.size(); }

private:
	void setupMesh();
};