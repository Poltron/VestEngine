#pragma once

#include "glm/fwd.hpp"

#include "Components/Entity.h"

template<typename T>
class ComponentManager;

struct HierarchyComponent;
struct LocalTransformComponent;
struct RigidbodyComponent;
struct WorldTransformComponent;

class PhysicsSystem
{
public:
	void update(ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<RigidbodyComponent>& inRigidbodies
		, double inDeltaTime);

private:
	void setWorldPosition(const glm::vec3& inPosition
		, LocalTransformComponent* inLocalTransform
		, WorldTransformComponent* inParentWorldTransform);

	void addWorldRotation(const glm::vec3& inRotation
		, LocalTransformComponent* inLocalTransform
		, WorldTransformComponent* inParentWorldTransform);

	void addWorldTranslation(const glm::vec3& inPosition
		, LocalTransformComponent* inLocalTransform
		, WorldTransformComponent* inParentWorldTransform);

	void setChildrenDirty(Entity inEntity
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<LocalTransformComponent>& inLocalTransforms);
};