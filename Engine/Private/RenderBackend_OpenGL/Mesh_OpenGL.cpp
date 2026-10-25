#include "RenderBackend_OpenGL/Mesh_OpenGL.h"

#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"

Mesh_OpenGL::Mesh_OpenGL(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: vertices(inVertices), indices(inIndices), textures(inTextures)
{
	VAO = {};
	VBO = {};
	EBO = {};

	setupMesh();
}

Mesh_OpenGL::~Mesh_OpenGL()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, VBO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, EBO);
}

Mesh_OpenGL::Mesh_OpenGL(Mesh_OpenGL&& inOther) noexcept
	: VAO(inOther.VAO), VBO(inOther.VBO), EBO(inOther.EBO)
{
	vertices.clear();
	indices.clear();
	textures.clear();

	vertices = inOther.vertices;
	indices = inOther.indices;
	textures = inOther.textures;

	inOther.VAO = {};
	inOther.VBO = {};
	inOther.EBO = {};
}

Mesh_OpenGL& Mesh_OpenGL::operator=(Mesh_OpenGL&& inOther) noexcept
{
	if (this != &inOther)
	{
		vertices.clear();
		indices.clear();
		textures.clear();

		vertices = inOther.vertices;
		indices = inOther.indices;
		textures = inOther.textures;

		VAO = inOther.VAO;
		VBO = inOther.VBO;
		EBO = inOther.EBO;

		inOther.VAO = {};
		inOther.VBO = {};
		inOther.EBO = {};

	}
	return *this;
}

void Mesh_OpenGL::setupMesh()
{
	VAO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTEX_ARRAY);
	VBO = render::getRenderBackend()->createGPUBuffer(EBufferType::VERTICES);

	// note : temporary for container mesh, to remove when only working with proper models
	if (indices.size() > 0)
	{
		EBO = render::getRenderBackend()->createGPUBuffer(EBufferType::INDICES);
	}

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTICES, VBO);
	render::getRenderBackend()->allocateGPUBuffer(EBufferType::VERTICES, vertices.size() * sizeof(Vertex), vertices.data(), EBufferUsage::STATIC);
	
	// note : temporary for container mesh, to remove when only working with proper models
	if (EBO.id > 0)
	{
		render::getRenderBackend()->bindGPUBuffer(EBufferType::INDICES, EBO);
		render::getRenderBackend()->allocateGPUBuffer(EBufferType::INDICES, indices.size() * sizeof(unsigned int), indices.data(), EBufferUsage::STATIC);
	}
	
	const unsigned int divisor = 0;
	render::getRenderBackend()->setVertexLayoutData(0, 3, sizeof(Vertex), (void*)offsetof(Vertex, position), divisor);
	render::getRenderBackend()->setVertexLayoutData(1, 3, sizeof(Vertex), (void*)offsetof(Vertex, normal), divisor);
	render::getRenderBackend()->setVertexLayoutData(2, 2, sizeof(Vertex), (void*)offsetof(Vertex, texCoords), divisor);

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
}

void Mesh_OpenGL::bindTextures(const ResourcesManager& inResourcesManager) const
{
	for (std::vector<ResourceHandle>::size_type i = 0; i < textures.size(); ++i)
	{
		const ResourceHandle& textureHandle = textures[i];
		if (!textureHandle.IsValid())
		{
			continue;
		}

		const Texture* texture = inResourcesManager.getTexture(textureHandle);
		if (!texture)
		{
			continue;
		}

		render::getRenderBackend()->bindTexture(texture->getHandle(), texture->getType(), i);
	}
}