#pragma once

struct HierarchyComponent;
struct LocalTransformComponent;
struct WorldTransformComponent;

template<typename T>
class ComponentManager;

class HierarchySystem
{
public:
	void update(ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies);
};