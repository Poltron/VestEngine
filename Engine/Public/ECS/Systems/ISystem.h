#pragma once

class Scene;

class ISystem
{
public:
	virtual void update(Scene& inScene, double inDeltaTime) = 0;
};
