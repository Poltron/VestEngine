#pragma once

#include "glm/fwd.hpp"

#define ENTITY_FIRST 1
#define ENTITY_MAX 10010

using Entity = glm::uint32_t;

namespace EntityFuncs
{
	// needed to set this INLINE, why ?
	inline bool isEntityValid(Entity inEntity)
	{
		return inEntity != 0;
	}
}