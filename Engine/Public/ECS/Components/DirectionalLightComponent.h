#pragma once

#include "glm/vec3.hpp"

#include "ECS/Entity.h"
#include "Render/Color.h"

struct DirectionalLightComponent
{
	Entity entity = 0;

	glm::vec3 color = color::white;
	float intensity = 1.0f;
};