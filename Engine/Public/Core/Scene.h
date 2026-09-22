#pragma once

#include "Camera.h"

#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/PointLightComponent.h"
#include "ECS/Components/RigidbodyComponent.h"
#include "ECS/Components/SphereBoundingVolumeComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "ECS/ComponentManager.h"

class Scene
{
	Camera camera; 
	Camera cameraDebug;

	EntityManager entities;

	ComponentManager<LocalTransformComponent> localTransformComponents;
	ComponentManager<WorldTransformComponent> worldTransformComponents;
	ComponentManager<HierarchyComponent> hierarchyComponents;
	
	ComponentManager<MeshRendererComponent> meshRendererComponents;
	ComponentManager<SphereBoundingVolumeComponent> sphereBoundingVolumeComponents;

	ComponentManager<RigidbodyComponent> rigidbodyComponents;

	ComponentManager<DirectionalLightComponent> directionalLightComponents;
	ComponentManager<PointLightComponent> pointLightComponents;

public:
	bool initialize();
	void shutdown();

	Camera& getCamera() { return camera; }
	Camera& getCameraDebug() { return cameraDebug; }

	const EntityManager& getEntityManager() { return entities; }

	ComponentManager<LocalTransformComponent>& getLocalTransformComponents() { return localTransformComponents; }
	const ComponentManager<LocalTransformComponent>& getLocalTransformComponents() const { return localTransformComponents; }
	ComponentManager<WorldTransformComponent>& getWorldTransformComponents() { return worldTransformComponents; }
	const ComponentManager<WorldTransformComponent>& getWorldTransformComponents() const { return worldTransformComponents; }
	ComponentManager<HierarchyComponent>& getHierarchyComponents() { return hierarchyComponents; }
	const ComponentManager<HierarchyComponent>& getHierarchyComponents() const { return hierarchyComponents; }

	ComponentManager<MeshRendererComponent>& getMeshRendererComponents() { return meshRendererComponents; }
	const ComponentManager<MeshRendererComponent>& getMeshRendererComponents() const { return meshRendererComponents; }
	ComponentManager<SphereBoundingVolumeComponent>& getSphereBoundingVolumeComponents() { return sphereBoundingVolumeComponents; }
	const ComponentManager<SphereBoundingVolumeComponent>& getSphereBoundingVolumeComponents() const { return sphereBoundingVolumeComponents; }

	ComponentManager<RigidbodyComponent>& getRigidbodyComponents() { return rigidbodyComponents; }
	const ComponentManager<RigidbodyComponent>& getRigidbodyComponents() const { return rigidbodyComponents; }

	ComponentManager<DirectionalLightComponent>& getDirectionalLightComponents() { return directionalLightComponents; }
	const ComponentManager<DirectionalLightComponent>& getDirectionalLightComponents() const { return directionalLightComponents; }
	ComponentManager<PointLightComponent>& getPointLightComponents() { return pointLightComponents; }
	const ComponentManager<PointLightComponent>& getPointLightComponents() const { return pointLightComponents; }

	LocalTransformComponent* addTransformTo(Entity inEntity
		, const glm::vec3& inPosition
		, const glm::quat& inRotation
		, const glm::vec3& inScale
		, HierarchyComponent* inParentHierarchy);

	MeshRendererComponent* addMeshRendererTo(Entity inEntity
		, ResourceHandle inModel
		, ResourceHandle inShader);

	SphereBoundingVolumeComponent* addSphereBoundingVolumeTo(Entity inEntity
		, float radius);

	DirectionalLightComponent* addDirectionalLightTo(Entity inEntity
		, const glm::vec3& inColor
		, float inIntensity);

	PointLightComponent* addPointLightTo(Entity inEntity
		, const glm::vec3& inColor
		, float inIntensity
		, float inConstant
		, float inLinear
		, float inQuadratic);

	RigidbodyComponent* addRigidbodyTo(Entity inEntity
		, const glm::vec3& inLinearVelocity
		, const glm::vec3& inAngularVelocity);

	Entity createRenderedModel(ResourceHandle inModel
		, ResourceHandle inShader
		, const glm::vec3& inPosition
		, const glm::quat& inRotation
		, const glm::vec3& inScale
		, float inSphereRadius
		, HierarchyComponent* inParentHierarchy = nullptr);
};