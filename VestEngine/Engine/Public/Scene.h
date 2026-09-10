#pragma once

#include "Components/DirectionalLightComponent.h"
#include "Components/HierarchyComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"

#include "Managers/ComponentManager.h"

class Scene
{
public:
	bool initialize();
	void shutdown();

	void loadPlaceholderScene();

	EntityManager entities;

	ComponentManager<LocalTransformComponent> localTransformComponents;
	ComponentManager<WorldTransformComponent> worldTransformComponents;
	ComponentManager<HierarchyComponent> hierarchyComponents;
	ComponentManager<RigidbodyComponent> rigidbodyComponents;
	ComponentManager<MeshRendererComponent> meshRendererComponents;
	ComponentManager<DirectionalLightComponent> directionalLightComponents;
	ComponentManager<PointLightComponent> pointLightComponents;
};