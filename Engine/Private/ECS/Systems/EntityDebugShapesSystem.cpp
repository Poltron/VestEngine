#include "ECS/Systems/EntityDebugShapesSystem.h"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/SphereColliderComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Render/DebugShapes.h"

void EntityDebugShapesSystem::update(EntityDebugShapeMeshInstances& inSphereInstances
	, Scene& inScene)
{
	for (size_t i = 0; i < inSphereInstances.instances.size(); ++i)
	{
		Entity entity = inSphereInstances.entities[i];
		WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(entity);
		ensure(worldTransform);
		SphereColliderComponent* sphereCollider = inScene.getSphereColliderComponents().get(entity);
		ensure(sphereCollider);

		DebugShapeMeshInstance& shapeMeshInstance = inSphereInstances.instances[i];
		shapeMeshInstance.model = glm::translate(worldTransform->model, sphereCollider->offset);
	}
}