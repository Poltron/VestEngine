#pragma once

struct EntityDebugShapeMeshInstances;
class Scene;

class EntityDebugShapesSystem
{
public:
	void update(EntityDebugShapeMeshInstances& inSphereInstances, Scene& inScene);
};