#include "Engine.h"

#include "GLFW/glfw3.h" // for time

#include "Utils/EntityFactory.h"

bool Engine::initialize()
{
	int width = 800;
	int height = 600;
	GLFWwindow* window = windowManager.createWindow(width, height);
	if (!window)
	{
		return false;
	}

	inputManager.initialize(window);
	inputHandler.initialize(&inputManager, &windowManager);

	renderer.initialize();

	camera.initialize(&inputManager);
	renderer.setActiveCamera(&camera);

	// resources
	ResourceHandle litShader = resourcesManager.loadShader("../Resources/Shaders/vertex.glsl", "../Resources/Shaders/lit_fragment.glsl");
	ResourceHandle unlitShader = resourcesManager.loadShader("../Resources/Shaders/vertex.glsl", "../Resources/Shaders/unlit_fragment.glsl");

	ResourceHandle containerTexture = resourcesManager.loadTexture("../Resources/Textures/container2.png", "diffuse");
	ResourceHandle containerSpecularTexture = resourcesManager.loadTexture("../Resources/Textures/container2_specular.png", "specular");

	std::vector<Vertex> vertices = Mesh::getNormalTextureCubeVertices();
	std::vector<unsigned int> indices;
	std::vector<ResourceHandle> textures = { containerTexture, containerSpecularTexture };
	Mesh mesh(std::move(vertices), std::move(indices), std::move(textures));

	std::vector<Mesh> meshes;
	meshes.push_back(std::move(mesh));

	ResourceHandle cubeModel = resourcesManager.createModel(std::move(meshes), "cube");
	//ResourceHandle bagModel = resourcesManager.loadModel("../Resources/Models/backpack/backpack.obj");

	// placeholder scene
	{
		Entity bagEntity = EntityFactory::createSceneBag(entityManager, localTransformComponents, worldTransformComponents, hierarchyComponents, rigidbodyComponents, meshRendererComponents, cubeModel, litShader);
		EntityFactory::createSceneCubes(entityManager, localTransformComponents, worldTransformComponents, hierarchyComponents, meshRendererComponents, cubeModel, litShader, bagEntity);
		EntityFactory::createSceneLights(entityManager, localTransformComponents, worldTransformComponents, meshRendererComponents, directionalLightComponents, pointLightComponents, cubeModel, unlitShader);
	}

	renderer.fillLightParameters(worldTransformComponents, pointLightComponents, directionalLightComponents);

	return true;
}

int Engine::launch()
{
	double lastFrame = glfwGetTime();

	while (!isShutdownRequested())
	{
		double currentFrame = glfwGetTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		inputManager.processInput(windowManager.getWindow(), deltaTime);

		camera.update(deltaTime);

		physicsSystem.update(localTransformComponents, worldTransformComponents, hierarchyComponents, rigidbodyComponents, deltaTime);
		transformSystem.update(localTransformComponents, worldTransformComponents, hierarchyComponents, deltaTime);

		renderer.clear();
		renderer.render(resourcesManager, worldTransformComponents, meshRendererComponents, currentFrame);
		renderer.swap(windowManager.getWindow());
	}
	
	shutdown();
	return 0;
}

void Engine::shutdown()
{
	windowManager.destroyWindow();
}

//
bool Engine::isShutdownRequested()
{
	return windowManager.shouldCloseWindow();
}