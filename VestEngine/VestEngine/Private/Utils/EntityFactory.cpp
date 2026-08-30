#include "Utils/EntityFactory.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/HierarchyComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/ResourcesManager.h"
#include "Systems/Renderer.h"

namespace EntityFactory
{
	LocalTransformComponent* addTransformTo(Entity inEntity
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale)
	{
		LocalTransformComponent* localTransformComponent = inLocalTransforms.create(inEntity);
		localTransformComponent->position = inPosition;
		localTransformComponent->rotation = inRotation;
		localTransformComponent->scale = inScale;

		WorldTransformComponent* worldTransformComponent = inWorldTransforms.create(inEntity);
		worldTransformComponent->model = localTransformComponent->getLocalModelMatrix();

		return localTransformComponent;
	}

	MeshRendererComponent* addMeshRendererTo(Entity inEntity
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		MeshRendererComponent* meshRendererComponent = inMeshRenderers.create(inEntity);
		meshRendererComponent->model = inModel;
		meshRendererComponent->shader = inShader;
		return meshRendererComponent;
	}

	DirectionalLightComponent* addDirectionalLightTo(Entity inEntity
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, const glm::vec3& inColor
		, float inIntensity)
	{
		DirectionalLightComponent* directionalLightComponent =  inDirectionalLights.create(inEntity);
		directionalLightComponent->color = inColor;
		directionalLightComponent->intensity = inIntensity;
		return directionalLightComponent;
	}

	PointLightComponent* addPointLightTo(Entity inEntity
		, ComponentManager<PointLightComponent>& inPointLights
		, const glm::vec3& inColor
		, float inIntensity
		, float inConstant
		, float inLinear
		, float inQuadratic)
	{
		PointLightComponent* pointLightComponent = inPointLights.create(inEntity);
		pointLightComponent->color = inColor;
		pointLightComponent->intensity = inIntensity;
		pointLightComponent->constant = inConstant;
		pointLightComponent->linear = inLinear;
		pointLightComponent->quadratic = inQuadratic;

		return pointLightComponent;
		
	}

	RigidbodyComponent* addRigidbodyTo(Entity inEntity
		, ComponentManager<RigidbodyComponent>& inRigidBodies
		, const glm::vec3& inLinearVelocity
		, const glm::vec3& inAngularVelocity)
	{
		RigidbodyComponent* rigidbodyComponent = inRigidBodies.create(inEntity);
		rigidbodyComponent->linearVelocity = inLinearVelocity;
		rigidbodyComponent->angularVelocity = inAngularVelocity;
		return rigidbodyComponent;
	}

	HierarchyComponent* addHierarchyTo(Entity inEntity
		, ComponentManager<HierarchyComponent>& inHierarchies)
	{
		HierarchyComponent* hierarchyComponent = inHierarchies.create(inEntity);
		return hierarchyComponent;
	}

	Entity createRenderedModel(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale)
	{
		Entity entity = inEntityManager.createEntity();
		addTransformTo(entity, inLocalTransforms, inWorldTransforms, inPosition, inRotation, inScale);
		addMeshRendererTo(entity, inMeshRenderers, inModel, inShader);

		return entity;
	}


	Entity createSceneBag(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<RigidbodyComponent>& inRigidbodies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		Entity entity = EntityFactory::createRenderedModel(inEntityManager
			, inLocalTransforms
			, inWorldTransforms
			, inMeshRenderers
			, inModel
			, inShader
			, glm::vec3(0.0f, 0.0f, -1.0f)
			, glm::vec3(0.0f, 0.0f, 0.0f)
			, glm::vec3(0.4f, 0.4f, 0.4f));

		addHierarchyTo(entity, inHierarchies);
		addRigidbodyTo(entity
			, inRigidbodies
			, glm::vec3(0.0f, 0.0f, 0.0f)
			, glm::vec3(0.0f, 2.0f, 0.0f));

		return entity;
	}

	void createSceneCubes(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<HierarchyComponent>& inHierarchies
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, Entity inParentEntity)
	{
		glm::vec3 cubePositions[] = {
			glm::vec3(1.5f,  -3.2f,  0.3f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
		};

		glm::vec3 cubeRotations[] = {
			glm::vec3(0, 0, 0),
			glm::vec3(0, 15, 50),
			glm::vec3(75, 20, 250),
			glm::vec3(10, 63, 84),
			glm::vec3(120, 0, 3),
			glm::vec3(14, 20, 1),
			glm::vec3(196, 178, 0),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 50, 0),
			glm::vec3(0, 0, 105)
		};

		glm::vec3 cubeScales[] = {
			glm::vec3(1.0f,  1.0f, 1.0f),
			glm::vec3(0.5f,  1.0f, 1.0f),
			glm::vec3(1.0f,  0.5f, 1.0f),
			glm::vec3(1.0f,  1.0f, 0.5f),
			glm::vec3(0.7f,  1.0f, 1.0f),
			glm::vec3(1.0f,  0.7f, 1.0f),
			glm::vec3(1.0f,  1.0f, 1.0f),
			glm::vec3(0.3f,  1.0f, 1.0f),
			glm::vec3(1.0f,  1.0f, 1.0f),
			glm::vec3(1.0f,  1.0f, 0.3f)
		};

		glm::vec3 cubeColors[] = {
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f),
			glm::vec3(1.0f, 0.5f, 0.31f)
		};
		
		HierarchyComponent* parentHierarchy = inHierarchies.get(inParentEntity);
		assert(parentHierarchy);

		Entity cubes[10];
		for (size_t i = 0; i < 10; ++i)
		{
			Entity cube = EntityFactory::createRenderedModel(inEntityManager
				, inLocalTransforms
				, inWorldTransforms
				, inMeshRenderers
				, inModel
				, inShader
				, cubePositions[i]
				, cubeRotations[i]
				, cubeScales[i]);

			HierarchyComponent* cubeHierarchy = addHierarchyTo(cube, inHierarchies);
			cubeHierarchy ->attachTo(parentHierarchy);

			MeshRendererComponent* meshRenderer = inMeshRenderers.get(cube);
			meshRenderer->shaderParameters.addVec3("objectColor", cubeColors[i]);

			cubes[i] = cube;
		}
	}

	void createSceneLights(EntityManager& inEntityManager
		, ComponentManager<LocalTransformComponent>& inLocalTransforms
		, ComponentManager<WorldTransformComponent>& inWorldTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		glm::vec3 lightScale(0.2f);

		Entity directionalLightID = EntityFactory::createRenderedModel(inEntityManager
			, inLocalTransforms
			, inWorldTransforms
			, inMeshRenderers
			, inModel
			, inShader
			, glm::vec3(0,0,0)
			, glm::vec3(180, 0, 0)
			, lightScale);

		glm::vec3 directionalLightColor(glm::vec3(1, 1, 1));

		MeshRendererComponent* directionalLightMeshRenderer = inMeshRenderers.get(directionalLightID);
		directionalLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
		directionalLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
		directionalLightMeshRenderer->shaderParameters.addVec3("objectColor", directionalLightColor);

		DirectionalLightComponent* directionalLightComponent = EntityFactory::addDirectionalLightTo(directionalLightID
			, inDirectionalLights
			, directionalLightColor
			, 0.5f);

		Entity pointLights[MAX_POINT_LIGHTS];

		glm::vec3 pointLightPositions[] = {
			glm::vec3(0.7f,  -1.0f,  0.0f),
			glm::vec3(0.0f,  1.0f,  0.0f),
			glm::vec3(-2.0f,  3.0f, -5.0f)
		};

		glm::vec3 pointLightColors[] = {
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		};

		for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
		{
			Entity pointLightID = EntityFactory::createRenderedModel(inEntityManager
				, inLocalTransforms
				, inWorldTransforms
				, inMeshRenderers
				, inModel
				, inShader
				, pointLightPositions[i]
				, glm::vec3(0, 0, 0)
				, lightScale);

			MeshRendererComponent* pointLightMeshRenderer = inMeshRenderers.get(pointLightID);
			pointLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
			pointLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
			pointLightMeshRenderer->shaderParameters.addVec3("objectColor", pointLightColors[i]);

			PointLightComponent* pointLight = EntityFactory::addPointLightTo(pointLightID
				, inPointLights
				, pointLightColors[i]
				, 3.0f /* intensity */
				, 1.0f /* constant */
				, 0.5f /* linear */
				, 0.2f /* quadratic */);

			pointLights[i] = pointLightID;
		}
	}
}