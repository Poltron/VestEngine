#pragma once
#include "ECS/Systems/ISystem.h"

template<typename T>
class ComponentManager;
class Scene;

class HierarchySystem : public ISystem
{
public:
	virtual void update(Scene& inScene, double inDeltaTime) override;
};