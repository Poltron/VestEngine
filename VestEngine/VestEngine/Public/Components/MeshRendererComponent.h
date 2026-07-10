#pragma once

struct MeshRendererComponent
{
	unsigned int entityID;

	GLuint shaderID;
	
	GLuint texture0ID;
	GLuint texture1ID;

	glm::vec3 lightColor;
	glm::vec3 objectColor;

	GLuint VAOID;
};