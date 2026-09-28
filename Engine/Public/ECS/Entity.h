#pragma once

#include "glm/fwd.hpp"

#define ENTITY_FIRST 0
#define ENTITY_MAX 2010
#define ENTITY_INVALID UINT64_MAX

using Entity = size_t;

namespace EntityFuncs
{
	// needed to set this INLINE, why ?
	inline bool isEntityValid(Entity inEntity)
	{
		return inEntity != ENTITY_INVALID;
	}
}