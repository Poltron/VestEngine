#include "ECS/Systems/TransformSystem.h"

#include "ECS/ComponentManager.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/TransformComponent.h"

void TransformSystem::update(ComponentManager<LocalTransformComponent>& inLocalTransforms, ComponentManager<WorldTransformComponent>& inWorldTransforms, ComponentManager<HierarchyComponent>& inHierarchies)
{
	// local / world / hierarchy = parallel arrays

	for (size_t i = 0; i < inLocalTransforms.size(); ++i)
	{
		LocalTransformComponent* localTransform = inLocalTransforms.at(i);
		assert(localTransform);

		if (!localTransform->isDirty())
		{
			continue;
		}

		WorldTransformComponent* worldTransform = inWorldTransforms.at(i);
		ensure(worldTransform);

		HierarchyComponent* hierarchy = inHierarchies.at(i);
		ensure(hierarchy);

		glm::mat4 parentWorldModel = glm::mat4(1.0f);
		if (EntityFuncs::isEntityValid(hierarchy->parent))
		{
			WorldTransformComponent* parentWorldTransform = inWorldTransforms.get(hierarchy->parent);
			assert(parentWorldTransform);

			parentWorldModel = parentWorldTransform->model;
		}

		worldTransform->model = parentWorldModel * localTransform->getLocalModelMatrix();
		localTransform->setDirty(false);
	}
}