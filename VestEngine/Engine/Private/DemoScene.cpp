#include "DemoScene.h"

#include "ECS/ComponentManager.h"
#include "ECS/Entity.h"
#include "ECS/HierarchyHelper.h"
#include "Core/Engine.h"
#include "Core/ResourceHandle.h"
#include "Core/ResourcesManager.h"
#include "Core/Scene.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Render/Renderer.h"

namespace
{
	Entity createScenePivot(Scene& inScene
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		Entity entity = inScene.createRenderedModel(inModel
			, inShader
			, glm::vec3(0.0f, 0.0f, -1.0f)
			, glm::quat(glm::radians(glm::vec3(45, 45, 45)))
			, glm::vec3(0.4f, 0.4f, 0.4f));

		inScene.addRigidbodyTo(entity
			, glm::vec3(0.0f, 0.0f, 0.0f)
			, glm::vec3(15.0f, 15.0f, 15.0f));

		return entity;
	}

	std::vector<Entity> createSceneCubes(Scene& inScene
		, ResourceHandle inModel
		, ResourceHandle inShader
		, Entity inParentEntity)
	{
		std::vector<Entity> entities;
		const int N = 10;

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
			glm::vec3(1, 1, 1),
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

		HierarchyComponent* parentHierarchy = inScene.getHierarchyComponents().get(inParentEntity);
		assert(parentHierarchy);

		for (size_t i = 0; i < N; ++i)
		{
			glm::quat rotation = glm::quat(glm::radians(cubeRotations[i]));

			Entity cube = inScene.createRenderedModel(inModel
				, inShader
				, cubePositions[i]
				, rotation
				, cubeScales[i]
				, parentHierarchy);

			MeshRendererComponent* meshRenderer = inScene.getMeshRendererComponents().get(cube);
			meshRenderer->shaderParameters.addVec3("material.objectColor", cubeColors[i]);

			entities.push_back(cube);
		}

		return entities;
	}

	void createSceneLights(Scene& inScene
		, ResourceHandle inModel
		, ResourceHandle inShader)
	{
		glm::vec3 lightScale(0.2f);
		glm::quat lightRotation = glm::quat(glm::radians(glm::vec3(45, 0, 0)));

		Entity directionalLightID = inScene.createRenderedModel(inModel
			, inShader
			, glm::vec3(0, 0, 0)
			, lightRotation
			, lightScale);

		glm::vec3 directionalLightColor(glm::vec3(1, 1, 1));

		MeshRendererComponent* directionalLightMeshRenderer = inScene.getMeshRendererComponents().get(directionalLightID);
		directionalLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
		directionalLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
		directionalLightMeshRenderer->shaderParameters.addVec3("material.objectColor", directionalLightColor);

		DirectionalLightComponent* directionalLightComponent = inScene.addDirectionalLightTo(directionalLightID
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

		for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i)
		{
			Entity pointLightID = inScene.createRenderedModel(inModel
				, inShader
				, pointLightPositions[i]
				, glm::quat()
				, lightScale);

			MeshRendererComponent* pointLightMeshRenderer = inScene.getMeshRendererComponents().get(pointLightID);
			pointLightMeshRenderer->shaderParameters.addTexture("material.diffuse", 0);
			pointLightMeshRenderer->shaderParameters.addTexture("material.specular", 0);
			pointLightMeshRenderer->shaderParameters.addVec3("material.objectColor", pointLightColors[i]);

			PointLightComponent* pointLight = inScene.addPointLightTo(pointLightID
				, pointLightColors[i]
				, 3.0f /* intensity */
				, 1.0f /* constant */
				, 0.5f /* linear */
				, 0.2f /* quadratic */);

			pointLights[i] = pointLightID;
		}
	}
}

namespace demoScene
{
	void loadCubesDemo(Scene& inScene)
	{
		const std::string WorkDirTMP = WORKDIR;

		const std::string containerPath = WorkDirTMP + "/Resources/Textures/container2.png";
		const std::string containerSpecularPath = WorkDirTMP + "/Resources/Textures/container2_specular.png";
		ResourceHandle containerTexture = engine::getResources()->loadTexture(containerPath, "diffuse");
		ResourceHandle containerSpecularTexture = engine::getResources()->loadTexture(containerSpecularPath, "specular");

		std::vector<Vertex> vertices = Mesh::getNormalTextureCubeVertices();
		std::vector<unsigned int> indices;
		std::vector<ResourceHandle> textures = { containerTexture, containerSpecularTexture };
		Mesh mesh(std::move(vertices), std::move(indices), std::move(textures));

		std::vector<Mesh> meshes;
		meshes.push_back(std::move(mesh));

		ResourceHandle cubeModel = engine::getResources()->createModel(std::move(meshes), "cube");

		Renderer* renderer = render::getRenderer();

		Entity parentEntity = createScenePivot(inScene, cubeModel, renderer->getLitShader());
		std::vector<Entity> childEntities = createSceneCubes(inScene, cubeModel, renderer->getLitShader(), parentEntity);
		createSceneLights(inScene, cubeModel, renderer->getSolidColorShader());

		// tmp inputs
		platform::getInputManager().registerKeyCallback(input::EKey::P
			, [scene = &inScene, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
			{
				if (inState != input::EInputState::PRESS)
				{
					return;
				}

				for (Entity childEntity : childEntities)
				{
					HierarchyComponent* hierarchy = scene->getHierarchyComponents().get(childEntity);
					if (!hierarchy)
					{
						continue;
					}

					if (EntityFuncs::isEntityValid(hierarchy->parent))
					{
						hierarchyHelper::detach(*scene, hierarchy, EAttachmentRules::KeepWorld);
					}
					else
					{
						HierarchyComponent* parentHierarchy = scene->getHierarchyComponents().get(parentEntity);
						hierarchyHelper::attachTo(*scene, hierarchy, parentHierarchy, EAttachmentRules::KeepWorld);
					}
				}
			});

		platform::getInputManager().registerKeyCallback(input::EKey::O
			, [scene = &inScene, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
			{
				if (inState != input::EInputState::PRESS)
				{
					return;
				}

				HierarchyComponent* firstHierarchy = scene->getHierarchyComponents().get(childEntities[0]);
				if (EntityFuncs::isEntityValid(firstHierarchy->parent))
				{
					hierarchyHelper::detach(*scene, firstHierarchy, EAttachmentRules::KeepWorld);
				}
				else
				{
					HierarchyComponent* parentHierarchy = scene->getHierarchyComponents().get(parentEntity);
					hierarchyHelper::attachTo(*scene, firstHierarchy, parentHierarchy, EAttachmentRules::KeepWorld);
				}
			});

		platform::getInputManager().registerKeyCallback(input::EKey::I
			, [scene = &inScene, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
			{
				if (inState != input::EInputState::PRESS)
				{
					return;
				}

				std::vector<Entity> parentedEntities;
				for (size_t i = 0; i < scene->getHierarchyComponents().size(); ++i)
				{
					HierarchyComponent* hierarchy = scene->getHierarchyComponents().at(i);
					if (hierarchy && hierarchy->parent)
					{
						parentedEntities.push_back(hierarchy->entity);
					}
				}

				if (parentedEntities.size() == 0)
				{
					return;
				}

				Entity randomEntity = parentedEntities[std::rand() % parentedEntities.size()];
				HierarchyComponent* hierarchy = scene->getHierarchyComponents().get(randomEntity);
				hierarchyHelper::detach(*scene, hierarchy, EAttachmentRules::KeepWorld);
			});
	}

	void load200animals(Scene& inScene)
	{
		inScene.getCamera().setPosition(glm::vec3(6.24083f, 2.63226f, 10.6785f));
		inScene.getCamera().setRotation(-127.13f, -26.1418f);

		const std::string WorkDirTMP = WORKDIR;

		const std::string animals[] =
		{
			"/Resources/Models/kenney-cube-pets/animal-bee.obj",
			"/Resources/Models/kenney-cube-pets/animal-caterpillar.obj",
			"/Resources/Models/kenney-cube-pets/animal-crab.obj",
			"/Resources/Models/kenney-cube-pets/animal-koala.obj",
			"/Resources/Models/kenney-cube-pets/animal-pig.obj",
			"/Resources/Models/kenney-cube-pets/animal-beaver.obj",
			"/Resources/Models/kenney-cube-pets/animal-elephant.obj",
			"/Resources/Models/kenney-cube-pets/animal-panda.obj",
			"/Resources/Models/kenney-cube-pets/animal-polar.obj",
			"/Resources/Models/kenney-cube-pets/animal-tiger.obj"
		};

		for (int i = 0; i < 10; ++i)
		{
			ResourceHandle model = engine::getResources()->loadModel(WorkDirTMP + animals[i]);

			for (int j = 0; j < 20; ++j)
			{
				const glm::vec3 pos = glm::vec3(j * 0.5f, 0, i * 1.0f);
				const glm::quat rot = glm::quat(glm::radians(glm::vec3(0,0,0)));
				const glm::vec3 scale = glm::vec3(0.2f);
				Entity entity = inScene.createRenderedModel(model, render::getRenderer()->getLitShader(), pos, rot, scale);

				MeshRendererComponent* meshRenderer = inScene.getMeshRendererComponents().get(entity);
				meshRenderer->bOutline = false;
			}
		}

		Renderer* renderer = render::getRenderer();
		ResourceHandle model = engine::getResources()->loadModel(WorkDirTMP + animals[0]);
		createSceneLights(inScene, model, renderer->getSolidColorShader());
	}
}