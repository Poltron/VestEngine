#pragma once

#include <vector>

#include "ECS/Entity.h"
#include "Render/GPUHandle.h"

struct RenderedItem
{
	Entity entity;
	GPUProxyHandle material;
	GPUProxyHandle mesh;
};

class Scene;

class FrustumCullingSystem
{
public:
	void update(Scene& inScene, double inDeltaTime, std::vector<RenderedItem>& outRenderedItems);
};