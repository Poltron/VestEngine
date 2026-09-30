#include "Core/Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"

#include "Core/Engine.h"
#include "Core/Maths.h"
#include "Core/ResourcesManager.h"
#include "ECS/HierarchyHelper.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Render/Color.h"
#include "Render/Renderer.h"
#include "Render/Debug/DebugShapeSubRenderer.h"

bool Scene::initialize()
{
	camera.initialize();
	cameraDebug.initialize();

	render::getRenderer()->setActiveCamera(&camera);

	localTransformComponents.setLabel("localTransforms");
	worldTransformComponents.setLabel("worldTransforms");
	hierarchyComponents.setLabel("hierarchies");
	rigidbodyComponents.setLabel("rigidbodies");
	meshRendererComponents.setLabel("meshRenderers");
	directionalLightComponents.setLabel("directionalLights");
	pointLightComponents.setLabel("pointLights");

	return true;
}

void Scene::shutdown()
{}

void Scene::executeEntityChanges()
{
	for (Entity entity : entities.getEntitiesPendingDestruction())
	{
		if (localTransformComponents.contains(entity))
		{
			localTransformComponents.destroy(entity);
		}
		if (worldTransformComponents.contains(entity))
		{
			worldTransformComponents.destroy(entity);
		}
		if (hierarchyComponents.contains(entity))
		{
			hierarchyComponents.destroy(entity);
		}
		if (meshRendererComponents.contains(entity))
		{
			meshRendererComponents.destroy(entity);
		}
		if (sphereColliderComponents.contains(entity))
		{
			sphereColliderComponents.destroy(entity);
		}
		if (rigidbodyComponents.contains(entity))
		{
			rigidbodyComponents.destroy(entity);
		}
		if (directionalLightComponents.contains(entity))
		{
			directionalLightComponents.destroy(entity);
		}
		if (pointLightComponents.contains(entity))
		{
			pointLightComponents.destroy(entity);
		}

		render::getRenderer()->getDebugShapeSubRenderer().removeMovableSphere(entity);
	}

	entities.destroyMarkedEntities();
}

bool Scene::raycast(const glm::vec3& inStart, const glm::vec3& inDirection, Entity& outHit) const
{
	struct Collision
	{
		Entity entity;
		float distance;
	};

	std::vector<Collision> collisions;

	for (size_t i = 0; i < sphereColliderComponents.size(); ++i)
	{
		const SphereColliderComponent* sphereCollider = sphereColliderComponents.at(i);
		ensure(sphereCollider);
		const WorldTransformComponent* worldTransform = worldTransformComponents.get(sphereCollider->entity);
		ensure(worldTransform);

		const glm::mat4 offsetModel = glm::translate(worldTransform->model, sphereCollider->offset);
		const glm::vec3 spherePosition = maths::getMatrixTranslation(offsetModel);
		render::getRenderer()->getDebugShapeSubRenderer().addPoint(spherePosition, 5.0f, color::red);
		
		const glm::vec3 sphereSquaredScale = maths::getMatrixSquaredScale(worldTransform->model);
		float maxScaleAxis = glm::sqrt(std::max({ sphereSquaredScale.x, sphereSquaredScale.y, sphereSquaredScale.z }));
		const float sphereSquaredRadius = (maxScaleAxis * sphereCollider->radius) * (maxScaleAxis * sphereCollider->radius);

		float intersectionDistance = 0;
		if (glm::intersectRaySphere<glm::vec3>(inStart, inDirection, spherePosition, sphereSquaredRadius, intersectionDistance))
		{
			Collision collision;
			collision.entity = sphereCollider->entity;
			collision.distance = intersectionDistance;
			collisions.push_back(collision);
		}
	}

	if (collisions.size() > 0)
	{
		Collision closest = collisions[0];
		for (size_t i = 1; i < collisions.size(); ++i)
		{
			const Collision& collision = collisions[i];
			if (collision.distance < closest.distance)
			{
				closest = collision;
			}
		}

		outHit = closest.entity;

		return true;
	}

	return false;
}

LocalTransformComponent* Scene::addTransformTo(Entity inEntity
	, const glm::vec3& inPosition
	, const glm::quat& inRotation
	, const glm::vec3& inScale
	, HierarchyComponent* inParentHierarchy)
{
	LocalTransformComponent* localTransformComponent = localTransformComponents.create(inEntity);
	localTransformComponent->setLocalPosition(inPosition);
	localTransformComponent->setLocalRotation(inRotation);
	localTransformComponent->setLocalScale(inScale);

	WorldTransformComponent* worldTransformComponent = worldTransformComponents.create(inEntity);
	worldTransformComponent->model = localTransformComponent->computeModel();

	HierarchyComponent* hierarchyComponent = hierarchyComponents.create(inEntity);
	if (inParentHierarchy)
	{
		hierarchyHelper::attachTo(*this, hierarchyComponent, inParentHierarchy, EAttachmentRules::KeepWorld);
	}

	return localTransformComponent;
}

MeshRendererComponent* Scene::addMeshRendererTo(Entity inEntity
	, ResourceHandle inModel
	, ResourceHandle inShader)
{
	MeshRendererComponent* meshRendererComponent = meshRendererComponents.create(inEntity);
	meshRendererComponent->model = inModel;
	meshRendererComponent->shader = inShader;
	meshRendererComponent->shaderParameters.addVec3("material.objectColor", color::white);
	return meshRendererComponent;
}

SphereColliderComponent* Scene::addSphereColliderTo(Entity inEntity
	, float inRadius
	, const glm::vec3& inOffset)
{
	SphereColliderComponent* sphereColliderComponent = sphereColliderComponents.create(inEntity);
	sphereColliderComponent->radius = inRadius;
	sphereColliderComponent->offset = inOffset;
	return sphereColliderComponent;
}

DirectionalLightComponent* Scene::addDirectionalLightTo(Entity inEntity
	, const glm::vec3& inColor
	, float inIntensity)
{
	DirectionalLightComponent* directionalLightComponent = directionalLightComponents.create(inEntity);
	directionalLightComponent->color = inColor;
	directionalLightComponent->intensity = inIntensity;
	return directionalLightComponent;
}

PointLightComponent* Scene::addPointLightTo(Entity inEntity
	, const glm::vec3& inColor
	, float inIntensity
	, float inConstant
	, float inLinear
	, float inQuadratic)
{
	PointLightComponent* pointLightComponent = pointLightComponents.create(inEntity);
	pointLightComponent->color = inColor;
	pointLightComponent->intensity = inIntensity;
	pointLightComponent->constant = inConstant;
	pointLightComponent->linear = inLinear;
	pointLightComponent->quadratic = inQuadratic;

	return pointLightComponent;

}

RigidbodyComponent* Scene::addRigidbodyTo(Entity inEntity
	, const glm::vec3& inLinearVelocity
	, const glm::vec3& inAngularVelocity)
{
	RigidbodyComponent* rigidbodyComponent = rigidbodyComponents.create(inEntity);
	rigidbodyComponent->linearVelocity = inLinearVelocity;
	rigidbodyComponent->angularVelocity = inAngularVelocity;
	return rigidbodyComponent;
}

Entity Scene::createRenderedModel(ResourceHandle inModel
	, ResourceHandle inShader
	, const glm::vec3& inPosition
	, const glm::quat& inRotation
	, const glm::vec3& inScale
	, HierarchyComponent* inParentHierarchy)
{
	Entity entity = entities.createEntity();
	addTransformTo(entity, inPosition, inRotation, inScale, inParentHierarchy);
	addMeshRendererTo(entity, inModel, inShader);
	return entity;
}