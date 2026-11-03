#include "ECS/Systems/EntityDebugShapesSystem.h"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/SphereColliderComponent.h"
#include "ECS/Components/TransformComponent.h"

void EntityDebugShapesSystem::update(Scene& inScene, double inDeltaTime)
{
	ensure(debugShapeMeshInstances);
	for (size_t i = 0; i < debugShapeMeshInstances->instances.size(); ++i)
	{
		Entity entity = debugShapeMeshInstances->entities[i];
		WorldTransformComponent* worldTransform = inScene.getWorldTransformComponents().get(entity);
		ensure(worldTransform);
		SphereColliderComponent* sphereCollider = inScene.getSphereColliderComponents().get(entity);
		ensure(sphereCollider);

		DebugShapeMeshInstance& shapeMeshInstance = debugShapeMeshInstances->instances[i];
		shapeMeshInstance.model = glm::translate(worldTransform->model, sphereCollider->offset);
	}
}