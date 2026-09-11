#pragma once

struct HierarchyComponent;
struct LocalTransformComponent;
struct WorldTransformComponent;

template<typename T>
class ComponentManager;

class TransformSystem
{
public:
	void update(ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies);
};
