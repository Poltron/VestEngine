#pragma once
#include "ECS/Systems/ISystem.h"

class TransformSystem : public ISystem
{
public:
	virtual void update(Scene& inScene, double inDeltaTime) override;
};
