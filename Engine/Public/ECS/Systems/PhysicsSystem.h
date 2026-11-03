#pragma once

#include "glm/fwd.hpp"

#include "ECS/Entity.h"
#include "ECS/Systems/ISystem.h"

struct HierarchyComponent;
struct LocalTransformComponent;
struct WorldTransformComponent;

template<typename T>
class ComponentManager;
class Scene;

class PhysicsSystem : public ISystem
{
public:
	virtual void update(Scene& inScene, double inDeltaTime) override;

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