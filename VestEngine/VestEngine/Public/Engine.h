#pragma once

#include "Camera.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/InputManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/WindowManager.h"
#include "Systems/Renderer.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TransformComponent.h"

class Engine
{
	ResourcesManager resourcesManager;
	InputManager inputManager;
	WindowManager windowManager;

	Renderer renderer;
	Camera camera;

	EntityManager entityManager;
	ComponentManager<TransformComponent> transformComponents;
	ComponentManager<MeshRendererComponent> meshRendererComponents;
	ComponentManager<DirectionalLightComponent> directionalLightComponents;
	ComponentManager<PointLightComponent> pointLightComponents;

public:
	bool initialize();
	int launch();
	void shutdown();

private:
	bool isShutdownRequested();
};

