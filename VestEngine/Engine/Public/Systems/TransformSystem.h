#pragma once

#include "Components/HierarchyComponent.h"
#include "Components/TransformComponent.h"

class TransformSystem
{
public:
	void update(ComponentManager<LocalTransformComponent>& inLocalTransforms, ComponentManager<WorldTransformComponent>& inWorldTransforms, ComponentManager<HierarchyComponent>& inHierarchies, double inDeltaTime)
	{
		// note: works currently because of components were created in a hierarchical depth order ( parent first ( bag ), then children ( cubes ) )
		// need to add sorting of hierarchies / localtransforms / worldtransforms so we can just do it in a single pass and every parent is already computed
		// 1/ modified the componentmanager's lookuptable so it links entityID to index instead of a direct ptr
		// 2/ now I can use std::sort() to sort the hierarchycomponent hierarchy array

		for (size_t i = 0; i < inLocalTransforms.size(); ++i)
		{
			LocalTransformComponent* localTransform = inLocalTransforms.at(i);
			assert(localTransform);

			if (!localTransform->bDirty)
			{
				continue;
			}

			WorldTransformComponent* worldTransform = inWorldTransforms.get(localTransform->entity);
			assert(worldTransform);

			if (!inHierarchies.contains(localTransform->entity))
			{
				worldTransform->model = localTransform->getLocalModelMatrix();
				localTransform->bDirty = false;
				continue;
			}

			HierarchyComponent* hierarchy = inHierarchies.get(localTransform->entity);
			glm::mat4 parentWorldModel = glm::mat4(1.0f);
			if (hierarchy->parent > 0)
			{
				WorldTransformComponent* parentWorldTransform = inWorldTransforms.get(hierarchy->parent);
				assert(parentWorldTransform);

				parentWorldModel = parentWorldTransform->model;
			}

			worldTransform->model = parentWorldModel * localTransform->getLocalModelMatrix();
			localTransform->bDirty = false;
		}
	}
};
