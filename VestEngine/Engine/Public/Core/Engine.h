#pragma once

#include "Camera.h"
#include "ECS/Systems/HierarchySystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/TransformSystem.h"

class Scene;
class ResourcesManager;



class Engine
{
public:
	enum class State
	{
		INITIALIZING,
		RUNNING,
		SHUTTING_DOWN,
		SHUTDOWN
	};

	State getState() { return state; }
	void setState(State inState) { state = inState; }

private:
	State state;

// todo: probably move all this 
protected:
	Camera camera;

	HierarchySystem hierarchySystem;
	PhysicsSystem physicsSystem;
	TransformSystem transformSystem;

};

namespace engine
{
	bool initialize();
	void launch();
	void shutdown();

	Engine* getEngine();
	Scene* getScene();
	ResourcesManager* getResources();
}