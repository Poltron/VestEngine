#pragma once

#include "glm/vec3.hpp"

#include "ECS/Entity.h"

struct RigidbodyComponent
{
	Entity entity;

	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;

	RigidbodyComponent()
		: entity(ENTITY_INVALID), linearVelocity(glm::vec3()), angularVelocity(glm::vec3())
	{}
};