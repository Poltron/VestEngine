#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "Resources/Texture.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	Vertex()
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
	// ---
public:
	//static void createTextureCube(Mesh& outMesh);
	//static void createNormalCube(Mesh& outMesh);
	static std::vector<Vertex> getNormalTextureCubeVertices();
	// ---

public:
	Mesh();
	~Mesh();

	void loadMesh(const char* inMesh);
	void loadMesh(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices, const std::vector<Texture>& inTextures);

	GLuint getVAO() const { return VAO; }
	GLuint getVBO() const { return VBO; }
	GLuint getEBO() const { return EBO; }

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	void setupMesh();
};