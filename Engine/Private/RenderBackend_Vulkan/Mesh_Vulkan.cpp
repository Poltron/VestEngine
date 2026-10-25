#include "RenderBackend_Vulkan/Mesh_Vulkan.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"

Mesh_Vulkan::Mesh_Vulkan(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures)
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
	// todo
}