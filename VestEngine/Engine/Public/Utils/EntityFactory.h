#pragma once

#include "Managers/EntityManager.h"

struct DirectionalLightComponent;
struct HierarchyComponent;
struct LocalTransformComponent;
struct MeshRendererComponent;
struct PointLightComponent;
struct RigidbodyComponent;
struct WorldTransformComponent;

struct ResourceHandle;

template<typename T>
class ComponentManager;

namespace EntityFactory
{
	LocalTransformComponent* addTransformTo(Entity inEntity
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale
		, HierarchyComponent* inParentHierarchy);

	MeshRendererComponent* addMeshRendererTo(Entity inEntity
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader);

	DirectionalLightComponent* addDirectionalLightTo(Entity inEntity
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, const glm::vec3& inColor
		, float inIntensity);

	PointLightComponent* addPointLightTo(Entity inEntity
		, ComponentManager<PointLightComponent>& inPointLights
		, const glm::vec3& inColor
		, float inIntensity
		, float inConstant
		, float inLinear
		, float inQuadratic);

	RigidbodyComponent* addRigidbodyTo(Entity inEntity
		, ComponentManager<RigidbodyComponent>& inRigidBodies
		, const glm::vec3& inLinearVelocity
		, const glm::vec3& inAngularVelocity);

	Entity createRenderedModel(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale
		, HierarchyComponent* inParentHierarchy = nullptr);

	Entity createSceneBag(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<RigidbodyComponent>& inRigidbodies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader);

	std::vector<Entity> createSceneCubes(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, Entity inParentEntity);

	void createSceneLights(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inModel
		, ResourceHandle inShader);
};