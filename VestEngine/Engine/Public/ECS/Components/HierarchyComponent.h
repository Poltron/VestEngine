#pragma once

#include "ECS/Entity.h"

struct HierarchyComponent
{ 
	Entity entity = 0;

	Entity parent = 0;
	Entity firstChild = 0;
	Entity nextSibling = 0;

	bool bDirty = false;
};

