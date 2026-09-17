#pragma once

#include "Camera.h"
#include "ECS/Systems/HierarchySystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/TransformSystem.h"

class Scene;
class ResourcesManager;


#ifndef WORKDIR
#define WORKDIR "D:/VestEngine/VestEngine/"
#endif


class Engine
{
public:
	enum class EState
	{
		INITIALIZING,
		RUNNING,
		SHUTTING_DOWN,
		SHUTDOWN
	};

	inline EState getState() { return state; }
	inline void setState(EState inState) { state = inState; }

	// todo: clarify tick groups
	using UIUpdateCallback = std::function<void()>;
	void registerUIUpdateCallback(UIUpdateCallback inCallback)
	{
		uiUpdateCallback = inCallback;
	}

protected:
	UIUpdateCallback uiUpdateCallback;

private:
	EState state;

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