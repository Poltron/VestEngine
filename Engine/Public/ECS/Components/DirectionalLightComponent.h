#pragma once

#include "glm/vec3.hpp"

#include "ECS/Entity.h"
#include "Render/Color.h"

struct DirectionalLightComponent
{
	Entity entity;

	glm::vec3 color;
	float intensity;

	DirectionalLightComponent()
		: entity(ENTITY_INVALID), color(color::white), intensity(1.0f)
	{}
};