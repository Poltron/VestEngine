#include "Scene.h"

#include "Engine.h"
#include "Helpers/HierarchyHelper.h"
#include "Managers/ResourcesManager.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Utils/EntityFactory.h"

bool Scene::initialize()
{
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

void Scene::loadPlaceholderScene()
{
	// resources
	ResourceHandle litShader = engine::getResources()->loadShader("../Resources/Shaders/vertex.glsl", "../Resources/Shaders/lit_fragment.glsl");
	ResourceHandle unlitShader = engine::getResources()->loadShader("../Resources/Shaders/vertex.glsl", "../Resources/Shaders/unlit_fragment.glsl");

	ResourceHandle containerTexture = engine::getResources()->loadTexture("../Resources/Textures/container2.png", "diffuse");
	ResourceHandle containerSpecularTexture = engine::getResources()->loadTexture("../Resources/Textures/container2_specular.png", "specular");

	std::vector<Vertex> vertices = Mesh::getNormalTextureCubeVertices();
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures = { containerTexture, containerSpecularTexture };
	Mesh mesh(std::move(vertices), std::move(indices), std::move(textures));

	std::vector<Mesh> meshes;
	meshes.push_back(std::move(mesh));

	ResourceHandle cubeModel = engine::getResources()->createModel(std::move(meshes), "cube");
	//ResourceHandle bagModel = resourcesManager.loadModel("../Resources/Models/backpack/backpack.obj");
	EntityManager& entityManager = engine::getScene()->entities;

	// placeholder scene
	Entity parentEntity = EntityFactory::createScenePivot(entityManager, localTransformComponents, worldTransformComponents, hierarchyComponents, rigidbodyComponents, meshRendererComponents, cubeModel, litShader);
	std::vector<Entity> childEntities = EntityFactory::createSceneCubes(entityManager, localTransformComponents, worldTransformComponents, hierarchyComponents, meshRendererComponents, cubeModel, litShader, parentEntity);
	EntityFactory::createSceneLights(entityManager, localTransformComponents, worldTransformComponents, hierarchyComponents, meshRendererComponents, directionalLightComponents, pointLightComponents, cubeModel, unlitShader);

	platform::getInputManager().registerKeyCallback(input::EKey::P
		, [hierarchies = &hierarchyComponents, localTransforms = &localTransformComponents, worldTransforms = &worldTransformComponents, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
		{
			if (inState != input::EInputState::PRESS)
			{
				return;
			}

			for (Entity childEntity : childEntities)
			{
				HierarchyComponent* hierarchy = hierarchies->get(childEntity);
				if (!hierarchy)
				{
					continue;
				}

				if (EntityFuncs::isEntityValid(hierarchy->parent))
				{
					hierarchyHelper::detach(hierarchy, EAttachmentRules::KeepWorld, *hierarchies, *localTransforms, *worldTransforms);
				}
				else
				{
					HierarchyComponent* parentHierarchy = hierarchies->get(parentEntity);
					hierarchyHelper::attachTo(hierarchy, parentHierarchy, EAttachmentRules::KeepWorld, *localTransforms, *worldTransforms);
				}
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::O
		, [hierarchies = &hierarchyComponents, localTransforms = &localTransformComponents, worldTransforms = &worldTransformComponents, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
		{
			if (inState != input::EInputState::PRESS)
			{
				return;
			}

			HierarchyComponent* firstHierarchy = hierarchies->get(childEntities[0]);
			if (EntityFuncs::isEntityValid(firstHierarchy->parent))
			{
				hierarchyHelper::detach(firstHierarchy, EAttachmentRules::KeepWorld, *hierarchies, *localTransforms, *worldTransforms);
			}
			else
			{
				HierarchyComponent* parentHierarchy = hierarchies->get(parentEntity);
				hierarchyHelper::attachTo(firstHierarchy, parentHierarchy, EAttachmentRules::KeepWorld, *localTransforms, *worldTransforms);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::I
		, [hierarchies = &hierarchyComponents, localTransforms = &localTransformComponents, worldTransforms = &worldTransformComponents, childEntities, parentEntity](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
		{
			if (inState != input::EInputState::PRESS)
			{
				return;
			}

			std::vector<Entity> parentedEntities;
			for (size_t i = 0; i < hierarchies->size(); ++i)
			{
				HierarchyComponent* hierarchy = hierarchies->at(i);
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
			HierarchyComponent* hierarchy = hierarchies->get(randomEntity);
			hierarchyHelper::detach(hierarchy, EAttachmentRules::KeepWorld, *hierarchies, *localTransforms, *worldTransforms);

		});

	renderer::getRenderer()->fillLightParameters(worldTransformComponents, pointLightComponents, directionalLightComponents);
}