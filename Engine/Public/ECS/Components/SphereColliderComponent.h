#pragma once

#include "glm/glm.hpp"

#include "ECS/Entity.h"

// note: they will be used as Conservative Bounding Sphere
// meaning if entity transform is non-uniform, the greater scale axis will be used

struct SphereColliderComponent
{
	Entity entity;

	float radius;
	glm::vec3 offset;
	bool bInFrustum;
};