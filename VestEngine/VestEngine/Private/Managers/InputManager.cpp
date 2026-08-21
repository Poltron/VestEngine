#include "Managers/InputManager.h"

#include "GLFW/glfw3.h"


void InputManager::initialize(GLFWwindow* inWindow)
{
	glfwSetInputMode(inWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	lastXPos = 400;
	lastYPos = 300;
}

void InputManager::processInput(GLFWwindow* inWindow)
{
	if (glfwGetKey(inWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(inWindow, true);
	}

	if (glfwGetKey(inWindow, GLFW_KEY_UP) == GLFW_PRESS)
	{
		verticalAxis = 1.0f;
	}
	else if (glfwGetKey(inWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		verticalAxis = -1.0f;
	}
	else
	{
		verticalAxis = 0.0f;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		horizontalAxis = 1.0f;
	}
	else if (glfwGetKey(inWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		horizontalAxis = -1.0f;
	}
	else
	{
		horizontalAxis = 0.0f;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_T) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(inWindow, GLFW_KEY_Y) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void InputManager::resetInputs()
{
	xOffset = 0.0f;
	yOffset = 0.0f;
	scrollOffset = 0.0f;

	horizontalAxis = 0.0f;
	verticalAxis = 0.0f;
}