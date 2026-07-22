#pragma once

#include "glm/vec3.hpp"

struct PointLightComponent
{
	unsigned int entityID = 0;

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;

	float constant = 0;
	float linear = 0;
	float quadratic = 0;
};