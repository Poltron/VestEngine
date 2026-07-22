
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

//
#include "Camera.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"
#include "Managers/EntityManager.h"
#include "Resources/Mesh.h"
#include "Resources/Texture.h"
#include "Systems/Renderer.h"
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

#define NB_POINT_LIGHTS 3

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
	renderer.setActiveCamera(&camera);

	GLuint litShaderID = renderer.addShader("D:/VestEngine/VestEngine/Resources/Shaders/vertex.glsl", "D:/VestEngine/VestEngine/Resources/Shaders/lit_fragment.glsl");
	GLuint unlitShaderID = renderer.addShader("D:/VestEngine/VestEngine/Resources/Shaders/vertex.glsl", "D:/VestEngine/VestEngine/Resources/Shaders/unlit_fragment.glsl");
	
	Mesh cubeMesh;
	Mesh::createNormalTextureCube(cubeMesh);

	Texture container2Texture = Texture("D:/VestEngine/VestEngine/Resources/Textures/container2.png ", GL_RGBA);
	Texture container2SpecularTexture = Texture("D:/VestEngine/VestEngine/Resources/Textures/container2_specular.png ", GL_RGBA);

	GLuint cubeVAOID = renderer.createVAO(cubeMesh);

	EntityManager entityManager;
	ComponentManager<TransformComponent> transformComponents;
	ComponentManager<MeshRendererComponent> meshRendererComponents;
	ComponentManager<DirectionalLightComponent> directionalLightComponents;
	ComponentManager<PointLightComponent> pointLightComponents;

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
		Entity ID = createRenderedEntity(entityManager, transformComponents, meshRendererComponents);
		
		TransformComponent* transform = transformComponents.get(ID);
		transform->position = cubePositions[i];
		transform->rotation = cubeRotations[i];
		transform->scale = cubeScales[i];

		MeshRendererComponent* meshRenderer = meshRendererComponents.get(ID);
		meshRenderer->VAOID = cubeVAOID;
		meshRenderer->shaderID = litShaderID;
		meshRenderer->texture0ID = container2Texture.GetTextureID();
		meshRenderer->texture1ID = container2SpecularTexture.GetTextureID();
		meshRenderer->objectColor = cubeColors[i];
		
		cubes[i] = ID;
	}

	glm::vec3 lightScale(0.2f);

	Entity directionalLightID = createDirectionalLight(entityManager, transformComponents, meshRendererComponents, directionalLightComponents);
	TransformComponent* directionalLightTransform = transformComponents.get(directionalLightID);
	directionalLightTransform->scale = lightScale;
	directionalLightTransform->rotation = glm::vec3(180,0,0);

	MeshRendererComponent* directionalLightMeshRenderer = meshRendererComponents.get(directionalLightID);
	directionalLightMeshRenderer->VAOID = cubeVAOID;
	directionalLightMeshRenderer->shaderID = unlitShaderID;
	directionalLightMeshRenderer->texture0ID = 0;
	directionalLightMeshRenderer->texture1ID = 0;
	directionalLightMeshRenderer->objectColor = glm::vec3(1.0f, 1.0f, 1.0f);

	DirectionalLightComponent* directionalLightComponent = directionalLightComponents.get(directionalLightID);
	directionalLightComponent->color = glm::vec3(1.0f, 1.0f, 1.0f);
	directionalLightComponent->intensity = 0.5f;

	Entity pointLights[NB_POINT_LIGHTS];

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

	for (int i = 0; i < NB_POINT_LIGHTS; ++i)
	{
		Entity ID = createPointLight(entityManager, transformComponents, meshRendererComponents, pointLightComponents);

		TransformComponent* pointLightTransform = transformComponents.get(ID);
		pointLightTransform->position = pointLightPositions[i];
		pointLightTransform->scale = lightScale;

		MeshRendererComponent* pointLightMeshRenderer = meshRendererComponents.get(ID);
		pointLightMeshRenderer->VAOID = cubeVAOID;
		pointLightMeshRenderer->shaderID = unlitShaderID;
		pointLightMeshRenderer->objectColor = pointLightColors[i];

		PointLightComponent* pointLight = pointLightComponents.get(ID);
		pointLight->color = pointLightColors[i];
		pointLight->intensity = 3.0f;
		pointLight->constant = 1.0f;
		pointLight->linear = 0.5f;
		pointLight->quadratic = 0.2f;

		pointLights[i] = ID;
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
		renderer.render(transformComponents, meshRendererComponents, pointLightComponents, directionalLightComponents, currentFrame);
		renderer.swap(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}