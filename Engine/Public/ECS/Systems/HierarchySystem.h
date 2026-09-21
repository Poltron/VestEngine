#pragma once

template<typename T>
class ComponentManager;
class Scene;

class HierarchySystem
{
public:
	void update(Scene& inScene);
};