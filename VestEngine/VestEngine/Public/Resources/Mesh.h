#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "Resources/ResourceHandle.h"
#include "Resources/Texture.h"

class ResourcesManager;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	Vertex()
		: position(), normal(), texCoords()
	{}

	Vertex(float inPositionX, float inPositionY, float inPositionZ, float inNormalX, float inNormalY, float inNormalZ, float inTexCoordsU, float inTexCoordsV)
	{
		position = { inPositionX, inPositionY, inPositionZ };
		normal = { inNormalX, inNormalY, inNormalZ };
		texCoords = { inTexCoordsU, inTexCoordsV };
	}
};

class Shader;

class Mesh
{
public:
	static std::vector<Vertex> getNormalTextureCubeVertices();

public:
	Mesh() = delete;
	Mesh(std::vector<Vertex>&& inVertices, std::vector<unsigned int>&& inIndices, std::vector<ResourceHandle>&& inTextures);
	~Mesh();
	
	Mesh(const Mesh& inOther) = delete;
	Mesh& operator=(const Mesh& inOther) = delete;

	Mesh(Mesh&& inOther) noexcept;
	Mesh& operator=(Mesh&& inOther) noexcept;

	void bindTextures(const ResourcesManager& inResourcesManager, const Shader& inShader) const;
	void draw() const;

	GLuint getVAO() const { return VAO; }
	GLuint getVBO() const { return VBO; }
	GLuint getEBO() const { return EBO; }

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	void setupMesh();
};