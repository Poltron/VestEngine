#include "Core/Scene.h"

#include "Core/Engine.h"
#include "Core/ResourcesManager.h"
#include "ECS/EntityFactory.h"
#include "ECS/HierarchyHelper.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Render/Renderer.h"

bool Scene::initialize()
{
	camera.initialize();
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
	worldTransformComponent->model = localTransformComponent->getLocalModelMatrix();

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
	return meshRendererComponent;
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