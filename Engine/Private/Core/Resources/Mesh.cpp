#include "Core/Resources/Mesh.h"

#include "glad/glad.h"
#include "glm/detail/type_vec3.hpp"

#include "Core/ResourcesManager.h"
#include "RenderBackend/RenderBackend.h"

std::vector<Vertex> Mesh::getNormalTextureCubeVertices()
{
	return {
		// positions          // normals           // texture coords
		{ -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f },
		{ 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f },
		{ 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f },
		{  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f },
		{ -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f },
		{ -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f },

		{ -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f },
		{ 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f },
		{ 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f },
		{ 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f },
		{ -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f },
		{ -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f },

		{ -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f },
		{ -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
		{ -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
		{ -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f },

		{ 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f },
		{ 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f },
		{ 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
		{ 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f },
		{ 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f },
		{ 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f },

		{ -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f },
		{ 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f },
		{ 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f },
		{ 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f },
		{ -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f },
		{ -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f },

		{ -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f },
		{ 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f },
		{ 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f },
		{ 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f },
		{ -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f },
		{ -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f }
	};
}

Mesh::Mesh(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures)
	: vertices(inVertices), indices(inIndices), textures(inTextures)
{
	VAO = {};
	VBO = {};
	EBO = {};

	setupMesh();
}

Mesh::~Mesh()
{
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTEX_ARRAY, VAO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::VERTICES, VBO);
	render::getRenderBackend()->deleteGPUBuffer(EBufferType::INDICES, EBO);
}

Mesh::Mesh(Mesh&& inOther) noexcept
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

Mesh& Mesh::operator=(Mesh&& inOther) noexcept
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

void Mesh::setupMesh()
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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// note : temporary for container mesh, to remove when only working with proper models
	if (EBO.id > 0)
	{
		render::getRenderBackend()->bindGPUBuffer(EBufferType::INDICES, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	render::getRenderBackend()->bindGPUBuffer(EBufferType::VERTEX_ARRAY, 0);
}

void Mesh::bindTextures(const ResourcesManager& inResourcesManager) const
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