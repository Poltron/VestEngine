#pragma once

#include "ECS/Entity.h"

struct SphereBoundingVolumeComponent
{
	Entity entity;

	float radius;
	bool bInFrustum;
};