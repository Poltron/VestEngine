#pragma once
#include "ECS/Systems/ISystem.h"

class Scene;

class RespawnSystem : public ISystem
{
public:
	virtual void update(Scene& inScene, double inDeltaTime) override;
};
