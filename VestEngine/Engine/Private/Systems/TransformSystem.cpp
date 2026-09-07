#include "Systems/TransformSystem.h"

#include "Components/HierarchyComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"

void TransformSystem::update(ComponentManager<LocalTransformComponent>& inLocalTransforms, ComponentManager<WorldTransformComponent>& inWorldTransforms, ComponentManager<HierarchyComponent>& inHierarchies)
{
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