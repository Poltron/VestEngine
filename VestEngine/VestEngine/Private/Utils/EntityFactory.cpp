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
	Entity createRenderedEntity(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers)
	{
		Entity entityID = inEntityManager.createEntity();

		inTransforms.create(entityID);
		inMeshRenderers.create(entityID);

		return entityID;
	}

	Entity createDirectionalLight(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights)
	{
		Entity entityID = inEntityManager.createEntity();

		inTransforms.create(entityID);
		inMeshRenderers.create(entityID);
		inDirectionalLights.create(entityID);

		return entityID;
	}

	Entity createPointLight(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<PointLightComponent>& inPointLights)
	{
		Entity entityID = inEntityManager.createEntity();

		inTransforms.create(entityID);
		inMeshRenderers.create(entityID);
		inPointLights.create(entityID);

		return entityID;
	}

	void createPlaceholderCubes(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ResourceHandle inMesh
		, ResourceHandle inDiffuseTexture
		, ResourceHandle inSpecularTexture
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
			Entity ID = EntityFactory::createRenderedEntity(inEntityManager, inTransforms, inMeshRenderers);

			TransformComponent* transform = inTransforms.get(ID);
			transform->position = cubePositions[i];
			transform->rotation = cubeRotations[i];
			transform->scale = cubeScales[i];

			MeshRendererComponent* meshRenderer = inMeshRenderers.get(ID);
			meshRenderer->mesh = inMesh;
			meshRenderer->shader = inShader;
			meshRenderer->shaderParameters.addTexture("material.diffuse", inDiffuseTexture.handle);
			meshRenderer->shaderParameters.addTexture("material.specular", inSpecularTexture.handle);
			meshRenderer->shaderParameters.addVec3("objectColor", cubeColors[i]);

			cubes[i] = ID;
		}
	}

	void createPlaceholderLights(EntityManager& inEntityManager
		, ComponentManager<TransformComponent>& inTransforms
		, ComponentManager<MeshRendererComponent>& inMeshRenderers
		, ComponentManager<DirectionalLightComponent>& inDirectionalLights
		, ComponentManager<PointLightComponent>& inPointLights
		, ResourceHandle inMesh
		, ResourceHandle inShader)
	{
		glm::vec3 lightScale(0.2f);

		Entity directionalLightID = EntityFactory::createDirectionalLight(inEntityManager, inTransforms, inMeshRenderers, inDirectionalLights);
		TransformComponent* directionalLightTransform = inTransforms.get(directionalLightID);
		directionalLightTransform->scale = lightScale;
		directionalLightTransform->rotation = glm::vec3(180, 0, 0);

		DirectionalLightComponent* directionalLightComponent = inDirectionalLights.get(directionalLightID);
		directionalLightComponent->color = glm::vec3(1.0f, 1.0f, 1.0f);
		directionalLightComponent->intensity = 0.5f;

		MeshRendererComponent* directionalLightMeshRenderer = inMeshRenderers.get(directionalLightID);
		directionalLightMeshRenderer->mesh = inMesh;
		directionalLightMeshRenderer->shader = inShader;
		//directionalLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
		//directionalLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
		directionalLightMeshRenderer->shaderParameters.addVec3("objectColor", directionalLightComponent->color);

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
			Entity ID = EntityFactory::createPointLight(inEntityManager, inTransforms, inMeshRenderers, inPointLights);

			TransformComponent* pointLightTransform = inTransforms.get(ID);
			pointLightTransform->position = pointLightPositions[i];
			pointLightTransform->scale = lightScale;

			MeshRendererComponent* pointLightMeshRenderer = inMeshRenderers.get(ID);
			pointLightMeshRenderer->mesh = inMesh;
			pointLightMeshRenderer->shader = inShader;
			//pointLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
			//pointLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
			pointLightMeshRenderer->shaderParameters.addVec3("objectColor", pointLightColors[i]);

			PointLightComponent* pointLight = inPointLights.get(ID);
			pointLight->color = pointLightColors[i];
			pointLight->intensity = 3.0f;
			pointLight->constant = 1.0f;
			pointLight->linear = 0.5f;
			pointLight->quadratic = 0.2f;

			pointLights[i] = ID;
		}
	}
}