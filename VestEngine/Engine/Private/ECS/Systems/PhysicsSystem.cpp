#include "ECS/Systems/PhysicsSystem.h"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/RigidbodyComponent.h"
#include "ECS/Components/TransformComponent.h"

void PhysicsSystem::update(Scene& inScene, double inDeltaTime)
{
	ComponentManager<RigidbodyComponent>& rigidbodies = inScene.getRigidbodyComponents();
	ComponentManager<LocalTransformComponent>& localTransforms = inScene.getLocalTransformComponents();
	ComponentManager<WorldTransformComponent>& worldTransforms = inScene.getWorldTransformComponents();
	ComponentManager<HierarchyComponent>& hierarchies = inScene.getHierarchyComponents();

	for (size_t i = 0; i < rigidbodies.size(); ++i)
	{
		RigidbodyComponent* rigidbody = rigidbodies.at(i);
		assert(rigidbody);

		LocalTransformComponent* localTransform = localTransforms.get(rigidbody->entity);
		const glm::vec3 rotationVelocity = rigidbody->angularVelocity * (float)inDeltaTime;
		const glm::vec3 translationVelocity = rigidbody->linearVelocity * (float)inDeltaTime;

		HierarchyComponent* hierarchy = hierarchies.get(rigidbody->entity);
		if (hierarchy && hierarchy->parent)
		{
			WorldTransformComponent* parentWorldTransform = worldTransforms.get(hierarchy->parent);
			addWorldRotation(rotationVelocity, localTransform, parentWorldTransform);
			addWorldTranslation(translationVelocity, localTransform, parentWorldTransform);

			setChildrenDirty(hierarchy->firstChild, hierarchies, localTransforms);
		}
		else
		{
			glm::quat newRot = localTransform->getRotation() * glm::quat(glm::radians(rotationVelocity));
			localTransform->setLocalRotation(newRot);
			glm::vec3 newPos = localTransform->getPosition() + translationVelocity;
			localTransform->setLocalPosition(newPos);
		}

		if (hierarchy)
		{
			setChildrenDirty(hierarchy->firstChild, hierarchies, localTransforms);
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


	glm::quat newRot = inLocalTransform->getRotation() * glm::quat(glm::radians(inRotation));
	inLocalTransform->setLocalRotation(newRot);
}

void PhysicsSystem::addWorldTranslation(const glm::vec3& inPosition, LocalTransformComponent* inLocalTransform, WorldTransformComponent* inParentWorldTransform)
{
	glm::mat4 invParent = glm::inverse(inParentWorldTransform->model);
	glm::vec3 localPosition = glm::vec3(invParent * glm::vec4(inPosition, 0.0f));

	inLocalTransform->setLocalPosition(inLocalTransform->getPosition() + localPosition);
}

void PhysicsSystem::setChildrenDirty(Entity inEntity, ComponentManager<HierarchyComponent>& inHierarchies, ComponentManager<LocalTransformComponent>& inLocalTransforms)
{
	// note : could infinite loop
	Entity nextChild = inEntity;
	while (EntityFuncs::isEntityValid(nextChild))
	{
		LocalTransformComponent* localTransform = inLocalTransforms.get(nextChild);
		assert(localTransform);
		localTransform->setDirty(true);

		HierarchyComponent* hierarchyComponent = inHierarchies.get(nextChild);
		assert(hierarchyComponent);
		nextChild = hierarchyComponent->nextSibling;
	}
}