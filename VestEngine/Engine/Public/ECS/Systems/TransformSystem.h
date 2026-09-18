#pragma once

template<typename T>
class ComponentManager;
class Scene;

class TransformSystem
{
public:
	void update(Scene& inScene);
};
