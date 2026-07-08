
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//
#include "Camera.h"
#include "Components/TransformComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Resources/Mesh.h"
#include "Resources/Texture.h"
#include "Systems/RendererSystem.h"
#include "Systems/TransformSystem.h"

Camera camera;

float verticalAxis = 0.0f;
float horizontalAxis = 0.0f;

const float mouseSensitivity = 10.0f;
float lastXPos = -1.0f;
float xOffset = 0.0f;
float lastYPos = -1.0f;
float yOffset = 0.0f;

const float scrollSensitivity = 100.0f;
float scrollOffset = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	if (lastXPos == -1.0f && lastYPos == -1.0f)
	{
		lastXPos = (float)xPos;
		lastYPos = (float)yPos;
	}

	xOffset = (float)xPos - lastXPos;
	yOffset = lastYPos - (float)yPos;
	lastXPos = (float)xPos;
	lastYPos = (float)yPos;

	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	// note : if not here, we skip a lot of inputs resulting in inconsistent movements
	// todo : move after adding proper input system
	camera.consumeMouseMovementInputs(xOffset, yOffset, 1.0 / 60.0);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	scrollOffset = (float)yOffset * scrollSensitivity;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		verticalAxis = 1.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		verticalAxis = -1.0f;
	}
	else
	{
		verticalAxis = 0.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		horizontalAxis = 1.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		horizontalAxis = -1.0f;
	}
	else
	{
		horizontalAxis = 0.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void resetInputs()
{
	xOffset = 0.0f;
	yOffset = 0.0f;
	scrollOffset = 0.0f;

	horizontalAxis = 0.0f;
	verticalAxis = 0.0f;
}

GLFWwindow* createWindow(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "VestEngine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	return window;
}

void setupInput(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	lastXPos = 400;
	lastYPos = 300;

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

Entity createCubeEntity(EntityManager& inEntityManager
	, ComponentManager<TransformComponent>& inTransforms
	, ComponentManager<MeshRendererComponent>& inMeshRenderers)
{
	Entity entityID = inEntityManager.createEntity();

	inTransforms.create(entityID);
	inMeshRenderers.create(entityID);

	return entityID;
}

int main()
{
	int width = 800;
	int height = 600;
	GLFWwindow* window = createWindow(width, height);
	if (!window)
	{
		return -1;
	}

	setupInput(window);
	
	glEnable(GL_DEPTH_TEST);

	Renderer renderer;
	GLuint shaderID = renderer.addShader("D:/VestEngine/VestEngine/Resources/Shaders/vertex.glsl", "D:/VestEngine/VestEngine/Resources/Shaders/fragment.glsl");
	
	Mesh mesh;
	Mesh::createCube(mesh);

	Texture containerTexture = Texture("D:/VestEngine/VestEngine/Resources/Textures/container.jpg", GL_RGB);
	Texture faceTexture = Texture("D:/VestEngine/VestEngine/Resources/Textures/awesomeface.png", GL_RGBA);

	GLuint VAOID = renderer.createVAO(mesh);

	EntityManager entityManager;
	ComponentManager<TransformComponent> transformComponents;
	ComponentManager<MeshRendererComponent> meshRendererComponents;

	glm::vec3 cubePositions[] = {
		glm::vec3(0.1f,  0.2f,  0.3f),
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

	Entity cubes[10];
	for (size_t i = 0; i < 10; ++i)
	{
		Entity ID = createCubeEntity(entityManager, transformComponents, meshRendererComponents);
		
		TransformComponent* transform = transformComponents.get(ID);
		transform->position = cubePositions[i];
		transform->rotation = cubeRotations[i];
		transform->scale = cubeScales[i];

		MeshRendererComponent* meshRenderer = meshRendererComponents.get(ID);
		meshRenderer->shaderID = shaderID;
		meshRenderer->texture0ID = containerTexture.GetTextureID();
		meshRenderer->texture1ID = faceTexture.GetTextureID();
		meshRenderer->VAOID = VAOID;
		
		cubes[i] = ID;
	}

	TransformSystem transformSystem;

	double lastFrame = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double currentFrame = glfwGetTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		camera.consumeKeyboardInputs(horizontalAxis, verticalAxis, deltaTime);
		camera.consumeMouseScrollInputs(scrollOffset, deltaTime);
		
		resetInputs();

		transformSystem.update(transformComponents);

		renderer.clear();
		renderer.render(transformComponents, meshRendererComponents, camera, currentFrame);
		renderer.swap(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}