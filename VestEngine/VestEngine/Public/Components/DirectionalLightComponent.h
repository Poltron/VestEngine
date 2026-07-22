#pragma once

#include "glm/vec3.hpp"

struct DirectionalLightComponent
{
	unsigned int entityID = 0;

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
};