#pragma once

#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/RigidbodyComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "ECS/ComponentManager.h"

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