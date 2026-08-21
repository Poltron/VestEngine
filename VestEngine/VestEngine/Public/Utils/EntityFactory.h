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
	Entity createRenderedEntity(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers);

	Entity createDirectionalLight(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights);

	Entity createPointLight(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<PointLightComponent>& inPointLights);

	void createPlaceholderCubes(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inMesh
		, ResourceHandle inDiffuseTexture
		, ResourceHandle inSpecularTexture
		, ResourceHandle inShader);

	void createPlaceholderLights(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inMesh
		, ResourceHandle inShader);
};