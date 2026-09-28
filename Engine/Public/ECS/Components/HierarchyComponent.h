#pragma once

#include "ECS/Entity.h"

struct HierarchyComponent
{ 
	Entity entity;

	Entity parent;
	Entity firstChild;
	Entity nextSibling;

	bool bDirty = false;

	HierarchyComponent()
		: entity(ENTITY_INVALID), parent(ENTITY_INVALID), firstChild(ENTITY_INVALID), nextSibling(ENTITY_INVALID)
	{}
};

