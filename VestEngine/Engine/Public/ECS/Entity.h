#pragma once

#include "glm/fwd.hpp"

using Entity = glm::uint32_t;

namespace EntityFuncs
{
	// needed to set this INLINE, why ?
	inline bool isEntityValid(Entity inEntity)
	{
		return inEntity != 0;
	}
}