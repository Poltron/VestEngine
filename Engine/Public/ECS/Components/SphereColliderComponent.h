#pragma once

#include "ECS/Entity.h"

struct SphereColliderComponent
{
	Entity entity;

	float radius;
	bool bInFrustum;
};