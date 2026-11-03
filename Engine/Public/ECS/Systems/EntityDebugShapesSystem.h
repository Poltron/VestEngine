#pragma once

#include "ECS/Systems/ISystem.h"
#include "Render/Debug/DebugShapes.h"

class Scene;

class EntityDebugShapesSystem : public ISystem
{
	EntityDebugShapeMeshInstances* debugShapeMeshInstances = nullptr;

public:
	void setMeshInstances(EntityDebugShapeMeshInstances* inDebugShapeMeshInstances) { debugShapeMeshInstances = inDebugShapeMeshInstances; }

	virtual void update(Scene& inScene, double inDeltaTime) override;
};