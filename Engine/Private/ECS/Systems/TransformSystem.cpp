#include "ECS/Systems/TransformSystem.h"

#include "tracy/Tracy.hpp"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/TransformComponent.h"

void TransformSystem::update(Scene& inScene)
{
	ZoneScoped;

	ComponentManager<LocalTransformComponent>& localTransforms = inScene.getLocalTransformComponents();
	ComponentManager<WorldTransformComponent>& worldTransforms = inScene.getWorldTransformComponents();
	ComponentManager<HierarchyComponent>& hierarchies = inScene.getHierarchyComponents();

	// local / world / hierarchy = parallel arrays

	for (size_t i = 0; i < localTransforms.size(); ++i)
	{
		LocalTransformComponent* localTransform = localTransforms.at(i);
		assert(localTransform);

		if (!localTransform->isDirty())
		{
			continue;
		}

		WorldTransformComponent* worldTransform = worldTransforms.at(i);
		ensure(worldTransform);

		HierarchyComponent* hierarchy = hierarchies.at(i);
		ensure(hierarchy);

		glm::mat4 parentWorldModel = glm::mat4(1.0f);
		if (EntityFuncs::isEntityValid(hierarchy->parent))
		{
			WorldTransformComponent* parentWorldTransform = worldTransforms.get(hierarchy->parent);
			assert(parentWorldTransform);

			parentWorldModel = parentWorldTransform->model;
		}

		worldTransform->model = parentWorldModel * localTransform->getLocalModelMatrix();
		localTransform->setDirty(false);
	}
}