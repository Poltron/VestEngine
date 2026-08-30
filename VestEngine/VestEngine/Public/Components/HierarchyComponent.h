#pragma once

#include "Components/Entity.h"

struct HierarchyComponent
{ 
	Entity entity = 0;

	Entity parent = 0;
	Entity firstChild = 0;
	Entity nextSibling = 0;

	void attachTo(HierarchyComponent* inParent)
	{
		parent = inParent->entity;

		nextSibling = inParent->firstChild;
		inParent->firstChild = entity;
	}
};

