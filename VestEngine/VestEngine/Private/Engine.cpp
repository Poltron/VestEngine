#include "Engine.h"

#include "GLFW/glfw3.h"

#include "Utils/EntityFactory.h"
#include "Systems/TransformSystem.h"

#define NB_POINT_LIGHTS 3

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

	camera.initialize(&inputManager);

	renderer.initialize();
	renderer.setActiveCamera(&camera);

	// resources
	ResourceHandle litShader = resourcesManager.loadShader("D:/VestEngine/VestEngine/Resources/Shaders/vertex.glsl", "D:/VestEngine/VestEngine/Resources/Shaders/lit_fragment.glsl");
	ResourceHandle unlitShader = resourcesManager.loadShader("D:/VestEngine/VestEngine/Resources/Shaders/vertex.glsl", "D:/VestEngine/VestEngine/Resources/Shaders/unlit_fragment.glsl");

	std::vector<Vertex> vertices = Mesh::getNormalTextureCubeVertices();
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	ResourceHandle cubeMesh = resourcesManager.loadMesh(vertices, indices, textures);

	ResourceHandle containerTexture = resourcesManager.loadTexture("D:/VestEngine/VestEngine/Resources/Textures/container2.png", GL_RGBA, "diffuse");
	ResourceHandle containerSpecularTexture = resourcesManager.loadTexture("D:/VestEngine/VestEngine/Resources/Textures/container2_specular.png", GL_RGBA, "specular");

	// placeholder scene
	EntityFactory::createPlaceholderCubes(entityManager, transformComponents, meshRendererComponents, cubeMesh, containerTexture, containerSpecularTexture, litShader);
	EntityFactory::createPlaceholderLights(entityManager, transformComponents, meshRendererComponents, directionalLightComponents, pointLightComponents, cubeMesh, unlitShader);

	renderer.fillLightParameters(transformComponents, pointLightComponents, directionalLightComponents);

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

		transformSystem.update(transformComponents, deltaTime);

		renderer.clear();
		renderer.render(resourcesManager, transformComponents, meshRendererComponents, currentFrame);
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