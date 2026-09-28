#include "ECS/Systems/KillZoneSystem.h"

#include "tracy/Tracy.hpp"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/TransformComponent.h"

void KillZoneSystem::update(Scene& inScene)
{
	ZoneScoped;

	ComponentManager<WorldTransformComponent>& worldTransforms = inScene.getWorldTransformComponents();

	for (size_t i = 0; i < worldTransforms.size(); ++i)
	{
		WorldTransformComponent* worldTransform = worldTransforms.at(i);
		assert(worldTransform);

		const glm::vec3 position = worldTransform->getPosition();
		if (position.x > 7.5f || position.x < -7.5f
			|| position.y > 7.5f || position.y < -7.5f
			|| position.z > 7.5f || position.z < -7.5f)
		{
			inScene.getEntityManager().markEntityForDestroy(worldTransform->entity);
		}
	}
}