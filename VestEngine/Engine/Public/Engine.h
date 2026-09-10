#pragma once

#include "Camera.h"

#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/UIManager.h"
#include "Platform/Platform.h"
#include "Systems/Renderer.h"
#include "Systems/HierarchySystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/TransformSystem.h"

class Scene;

class Engine
{
protected:
	UIManager uiManager;

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