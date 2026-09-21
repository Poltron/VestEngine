#pragma once

#include "glm/vec3.hpp"

#include "ECS/Entity.h"

struct RigidbodyComponent
{
	Entity entity = 0;

	glm::vec3 linearVelocity = glm::vec3(0);
	glm::vec3 angularVelocity = glm::vec3(0);
};