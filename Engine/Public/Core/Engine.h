#pragma once

#include "Camera.h"
#include "ECS/Systems/FrustumCullingSystem.h"
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
	HierarchySystem hierarchySystem;
	PhysicsSystem physicsSystem;
	TransformSystem transformSystem;
	FrustumCullingSystem frustumCullingSystem;
};

namespace engine
{
	bool initialize();
	Scene* createScene();
	void launch();
	void shutdown();

	Engine* getEngine();
	Scene* getScene();
	ResourcesManager* getResources();
}