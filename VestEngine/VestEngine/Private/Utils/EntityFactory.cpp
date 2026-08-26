#include "Utils/EntityFactory.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Managers/ResourcesManager.h"
#include "Systems/Renderer.h"

namespace EntityFactory
{
	DirectionalLightComponent* addDirectionalLightTo(Entity inID, ComponentManager<DirectionalLightComponent>& inDirectionalLights)
	{
		return inDirectionalLights.create(inID);
	}

	PointLightComponent* addPointLightTo(Entity inID, ComponentManager<PointLightComponent>& inPointLights)
	{
		return inPointLights.create(inID);
	}

	Entity createRenderedModel(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader
		, const glm::vec3& inPosition
		, const glm::vec3& inRotation
		, const glm::vec3& inScale)
	{
		Entity entityID = inEntityManager.createEntity();

		inTransforms.create(entityID);
		inMeshRenderers.create(entityID);

		MeshRendererComponent* meshRenderer = inMeshRenderers.get(entityID);
		meshRenderer->model = inModel;
		meshRenderer->shader = inShader;

		TransformComponent* transformComponent = inTransforms.get(entityID);
		transformComponent->position = inPosition;
		transformComponent->rotation = inRotation;
		transformComponent->scale = inScale;

		return entityID;
	}

	void createPlaceholderCubes(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inModel
		, ResourceHandle inShader)
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

		Entity cubes[10];
		for (size_t i = 0; i < 10; ++i)
		{
			Entity ID = EntityFactory::createRenderedModel(inEntityManager
				, inTransforms
				, inMeshRenderers
				, inModel
				, inShader
				, cubePositions[i]
				, cubeRotations[i]
				, cubeScales[i]);

			MeshRendererComponent* meshRenderer = inMeshRenderers.get(ID);
			meshRenderer->shaderParameters.addVec3("objectColor", cubeColors[i]);

			cubes[i] = ID;
		}
	}

	void createPlaceholderLights(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		glm::vec3 lightScale(0.2f);

		Entity directionalLightID = EntityFactory::createRenderedModel(inEntityManager
			, inTransforms
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

		DirectionalLightComponent* directionalLightComponent = EntityFactory::addDirectionalLightTo(directionalLightID, inDirectionalLights);
		directionalLightComponent->color = directionalLightColor;
		directionalLightComponent->intensity = 0.5f;

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
				, inTransforms
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

			PointLightComponent* pointLight = EntityFactory::addPointLightTo(pointLightID, inPointLights);
			pointLight->color = pointLightColors[i];
			pointLight->intensity = 3.0f;
			pointLight->constant = 1.0f;
			pointLight->linear = 0.5f;
			pointLight->quadratic = 0.2f;

			pointLights[i] = pointLightID;
		}
	}
}