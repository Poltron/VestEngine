#pragma once

#include "Managers/EntityManager.h"

struct DirectionalLightComponent;
struct PointLightComponent;
struct MeshRendererComponent;
struct TransformComponent;

struct ResourceHandle;

template<typename T>
class ComponentManager;

namespace EntityFactory
{
	DirectionalLightComponent* addDirectionalLightTo(Entity inID
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights);

	PointLightComponent* addPointLightTo(Entity inID
		, ComponentManager<PointLightComponent>& inPointLights);

	Entity createRenderedModel(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale);

	void createPlaceholderCubes(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader);

	void createPlaceholderLights(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inModel
		, ResourceHandle inShader);
};