#include "Systems/PhysicsSystem.h"

#include "Components/HierarchyComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"

void PhysicsSystem::update(ComponentManager<LocalTransformComponent>& inLocalTransforms
	, ComponentManager<WorldTransformComponent>& inWorldTransforms
	, ComponentManager<HierarchyComponent>& inHierarchies
	, ComponentManager<RigidbodyComponent>& inRigidbodies
	, double inDeltaTime)
{
	for (size_t i = 0; i < inRigidbodies.size(); ++i)
	{
		RigidbodyComponent* rigidbody = inRigidbodies.at(i);
		assert(rigidbody);

		LocalTransformComponent* localTransform = inLocalTransforms.get(rigidbody->entity);
		const glm::vec3 rotation = rigidbody->angularVelocity * (float)inDeltaTime;
		const glm::vec3 translation = rigidbody->linearVelocity * (float)inDeltaTime;

		HierarchyComponent* hierarchy = inHierarchies.get(rigidbody->entity);
		if (hierarchy && hierarchy->parent)
		{
			WorldTransformComponent* parentWorldTransform = inWorldTransforms.get(hierarchy->parent);
			addWorldRotation(rotation, localTransform, parentWorldTransform);
			addWorldTranslation(translation, localTransform, parentWorldTransform);

			setChildrenDirty(hierarchy->firstChild, inHierarchies, inLocalTransforms);
		}
		else
		{
			localTransform->setLocalRotation(localTransform->rotation + rotation);
			localTransform->setLocalPosition(localTransform->position + translation);
		}

		if (hierarchy)
		{
			setChildrenDirty(hierarchy->firstChild, inHierarchies, inLocalTransforms);
		}
	}
}

void PhysicsSystem::setWorldPosition(const glm::vec3& inPosition, LocalTransformComponent* inLocalTransform, WorldTransformComponent* inParentWorldTransform)
{
	glm::mat4 invParent = glm::inverse(inParentWorldTransform->model);
	glm::vec3 localPosition = glm::vec3(invParent * glm::vec4(inPosition, 0.0f));

	inLocalTransform->setLocalPosition(localPosition);
}

void PhysicsSystem::addWorldRotation(const glm::vec3& inRotation, LocalTransformComponent* inLocalTransform, WorldTransformComponent* inParentWorldTransform)
{
	glm::mat4 invParent = glm::inverse(inParentWorldTransform->model);
	glm::vec3 localRotation = glm::mat3(invParent) * inRotation;

	inLocalTransform->setLocalRotation(inLocalTransform->rotation + localRotation);
}

void PhysicsSystem::addWorldTranslation(const glm::vec3& inPosition, LocalTransformComponent* inLocalTransform, WorldTransformComponent* inParentWorldTransform)
{
	glm::mat4 invParent = glm::inverse(inParentWorldTransform->model);
	glm::vec3 localPosition = glm::vec3(invParent * glm::vec4(inPosition, 0.0f));

	inLocalTransform->setLocalPosition(inLocalTransform->position + localPosition);
}

void PhysicsSystem::setChildrenDirty(Entity inEntity, ComponentManager<HierarchyComponent>& inHierarchies, ComponentManager<LocalTransformComponent>& inLocalTransforms)
{
	// note : could infinite loop
	Entity nextChild = inEntity;
	while (EntityFuncs::isEntityValid(nextChild))
	{
		LocalTransformComponent* localTransform = inLocalTransforms.get(nextChild);
		assert(localTransform);
		localTransform->bDirty = true;

		HierarchyComponent* hierarchyComponent = inHierarchies.get(nextChild);
		assert(hierarchyComponent);
		nextChild = hierarchyComponent->nextSibling;
	}
}