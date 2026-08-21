#include "Managers/WindowManager.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Managers/InputManager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
	if (inputManager->lastXPos == -1.0f && inputManager->lastYPos == -1.0f)
	{
		inputManager->lastXPos = (float)xPos;
		inputManager->lastYPos = (float)yPos;
	}

	inputManager->xOffset = (float)xPos - inputManager->lastXPos;
	inputManager->yOffset = inputManager->lastYPos - (float)yPos;
	inputManager->lastXPos = (float)xPos;
	inputManager->lastYPos = (float)yPos;

	inputManager->xOffset *= inputManager->mouseSensitivity;
	inputManager->yOffset *= inputManager->mouseSensitivity;

	// note : if not here, we skip a lot of inputs resulting in inconsistent movements
	// todo : move after adding proper input system
	//camera.consumeMouseMovementInputs(inputManager->xOffset, inputManager->yOffset, 1.0 / 60.0);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	InputManager* inputManager = (InputManager*)glfwGetWindowUserPointer(window);
	inputManager->scrollOffset = (float)yOffset * inputManager->scrollSensitivity;
}

GLFWwindow* WindowManager::createWindow(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "VestEngine", NULL, NULL);
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

	glViewport(0, 0, width, height);

	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	return window;
}

void WindowManager::destroyWindow()
{
	glfwTerminate();
}

bool WindowManager::shouldCloseWindow() const
{
	return glfwWindowShouldClose(window);
}