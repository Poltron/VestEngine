#pragma once

#include "Components/Entity.h"

template<typename T>
class ComponentManager;

struct HierarchyComponent;

namespace hierarchyHelper
{
	void attachTo(HierarchyComponent* inElement, HierarchyComponent* inParent);
	void attachTo(HierarchyComponent* inElement, Entity inParent, ComponentManager<HierarchyComponent>& inHierarchies);
	void detach(HierarchyComponent* inElement, ComponentManager<HierarchyComponent>& inHierarchies);
}