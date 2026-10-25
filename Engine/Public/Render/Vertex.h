#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

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