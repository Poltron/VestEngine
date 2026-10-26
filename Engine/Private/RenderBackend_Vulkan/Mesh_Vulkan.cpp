#include "RenderBackend_Vulkan/Mesh_Vulkan.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"

Mesh_Vulkan::Mesh_Vulkan(std::vector<Vertex>&& inVertices, std::vector<uint16_t>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: vertices(inVertices), indices(inIndices), textures(inTextures)
{
	vertexBuffer = {};
	indexBuffer = {};

	setupMesh();
}

Mesh_Vulkan::~Mesh_Vulkan()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, vertexBuffer);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, indexBuffer);
}

Mesh_Vulkan::Mesh_Vulkan(Mesh_Vulkan&& inOther) noexcept
	: vertexBuffer(inOther.vertexBuffer), indexBuffer(inOther.indexBuffer)
{
	vertices.clear();
	indices.clear();
	textures.clear();

	vertices = inOther.vertices;
	indices = inOther.indices;
	textures = inOther.textures;

	inOther.vertexBuffer = {};
	inOther.indexBuffer = {};
}

Mesh_Vulkan& Mesh_Vulkan::operator=(Mesh_Vulkan&& inOther) noexcept
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

		inOther.vertexBuffer = {};
		inOther.indexBuffer = {};

	}
	return *this;
}

void Mesh_Vulkan::setupMesh()
{
	RenderDevice& renderDevice = render::getRenderBackend()->getDevice();
	size_t verticesSize = sizeof(Vertex) * vertices.size();
	vertexBuffer = renderDevice.createAndFillBuffer(verticesSize, vertices.data(), EBufferType::VERTICES);

	std::vector<uint16_t> indices = static_mesh::getPlaneIndices();
	size_t indicesSize = sizeof(uint16_t) * indices.size();
	indexBuffer = renderDevice.createAndFillBuffer(indicesSize, indices.data(), EBufferType::INDICES);
}