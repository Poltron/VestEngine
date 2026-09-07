#include "Helpers/HierarchyHelper.h"

#include "Components/HierarchyComponent.h"
#include "Managers/ComponentManager.h"

namespace hierarchyHelper
{
	void attachTo(HierarchyComponent* inElement, HierarchyComponent* inParent)
	{
		inElement->parent = inParent->entity;

		inElement->nextSibling = inParent->firstChild;
		inParent->firstChild = inElement->entity;
	}

	void attachTo(HierarchyComponent* inElement, Entity inParent, ComponentManager<HierarchyComponent>& inHierarchies)
	{
		HierarchyComponent* parentHierarchy = inHierarchies.get(inParent);
		attachTo(inElement, parentHierarchy);
	}

	void detach(HierarchyComponent* inElement, ComponentManager<HierarchyComponent>& inHierarchies)
	{
		if (!EntityFuncs::isEntityValid(inElement->parent))
		{
			return;
		}

		HierarchyComponent* parentHierarchy = inHierarchies.get(inElement->parent);
		ensure(parentHierarchy);

		if (parentHierarchy->firstChild == inElement->entity)
		{
			parentHierarchy->firstChild = inElement->nextSibling;

			inElement->parent = 0;
			inElement->nextSibling = 0;
			return;
		}

		HierarchyComponent* previousSiblingComponent = inHierarchies.get(parentHierarchy->firstChild);
		ensure(previousSiblingComponent);

		while (previousSiblingComponent->nextSibling != inElement->entity && EntityFuncs::isEntityValid(previousSiblingComponent->nextSibling))
		{
			previousSiblingComponent = inHierarchies.get(previousSiblingComponent->nextSibling);
		}

		ensure(previousSiblingComponent);
		ensure(previousSiblingComponent->nextSibling == inElement->entity);

		previousSiblingComponent->nextSibling = inElement->nextSibling;
		inElement->parent = 0;
		inElement->nextSibling = 0;
	}
}