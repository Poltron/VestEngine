#pragma once

#include "ECS/Entity.h"

// note: they will be used as Conservative Bounding Sphere
// meaning if entity transform is non-uniform, the greater scale axis will be used

struct SphereColliderComponent
{
	Entity entity;

	float radius;
	bool bInFrustum;
};