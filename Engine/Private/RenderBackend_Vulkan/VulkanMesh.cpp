#include "RenderBackend_Vulkan/VulkanMesh.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"

VulkanMesh::VulkanMesh(std::vector<Vertex>&& inVertices, std::vector<uint16_t>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: vertices(inVertices), indices(inIndices), textures(inTextures), vertexBuffer(GPUHandle::invalid), indexBuffer(GPUHandle::invalid)
{
	setupMesh();
}

VulkanMesh::~VulkanMesh()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, vertexBuffer);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, indexBuffer);
}

VulkanMesh::VulkanMesh(VulkanMesh&& inOther) noexcept
	: vertexBuffer(inOther.vertexBuffer), indexBuffer(inOther.indexBuffer)
{
	vertices.clear();
	indices.clear();
	textures.clear();

	vertices = inOther.vertices;
	indices = inOther.indices;
	textures = inOther.textures;

	inOther.vertexBuffer = GPUHandle::invalid;
	inOther.indexBuffer = GPUHandle::invalid;
}

VulkanMesh& VulkanMesh::operator=(VulkanMesh&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertices.clear();
		indices.clear();
		textures.clear();

		vertices = inOther.vertices;
		indices = inOther.indices;
		textures = inOther.textures;

		vertexBuffer = inOther.vertexBuffer;
		indexBuffer = inOther.indexBuffer;

		inOther.vertexBuffer = GPUHandle::invalid;
		inOther.indexBuffer = GPUHandle::invalid;
	}
	return *this;
}

void VulkanMesh::setupMesh()
{
	VulkanDevice& VulkanDevice = render::getRenderBackend()->getDevice();

	size_t verticesSize = sizeof(Vertex) * vertices.size();
	vertexBuffer = VulkanDevice.createAndFillBuffer(verticesSize, vertices.data(), EBufferType::VERTICES);

	size_t indicesSize = sizeof(uint16_t) * indices.size();
	indexBuffer = VulkanDevice.createAndFillBuffer(indicesSize, indices.data(), EBufferType::INDICES);
}