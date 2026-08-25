#include "Managers/WindowManager.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Managers/InputManager.h"

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