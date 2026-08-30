#pragma once

#include "Camera.h"

#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/InputHandler.h"
#include "Managers/InputManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/WindowManager.h"
#include "Systems/Renderer.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/TransformSystem.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/HierarchyComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"

class Engine
{
	ResourcesManager resourcesManager;
	InputHandler inputHandler;
	InputManager inputManager;
	WindowManager windowManager;

	Renderer renderer;
	Camera camera;

	EntityManager entityManager;

	ComponentManager<LocalTransformComponent> localTransformComponents;
	ComponentManager<WorldTransformComponent> worldTransformComponents;
	ComponentManager<HierarchyComponent> hierarchyComponents;
	ComponentManager<RigidbodyComponent> rigidbodyComponents;

	ComponentManager<MeshRendererComponent> meshRendererComponents;
	ComponentManager<DirectionalLightComponent> directionalLightComponents;
	ComponentManager<PointLightComponent> pointLightComponents;

	PhysicsSystem physicsSystem;
	TransformSystem transformSystem;

public:
	bool initialize();
	int launch();
	void shutdown();

private:
	bool isShutdownRequested();
};

