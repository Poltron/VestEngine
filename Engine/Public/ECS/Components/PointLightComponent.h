#pragma once

#include "glm/vec3.hpp"

#include "ECS/Entity.h"
#include "Render/Color.h"

struct PointLightComponent
{
	Entity entity = 0;

	glm::vec3 color = color::white;
	float intensity = 1.0f;

	float constant = 0;
	float linear = 0;
	float quadratic = 0;
};