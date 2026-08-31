#include "Managers/InputHandler.h"

#include <iostream>

#include "GLFW/glfw3.h"

#include "Managers/InputManager.h"
#include "Managers/WindowManager.h"

namespace WindowEvents
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}

namespace InputCallbacks
{
	void cursorPos_callback(GLFWwindow* window, double xPos, double yPos)
	{
		InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
		inputHandler->cursorPosCallback(xPos, yPos);
	}

	void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
	{
		InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
		inputHandler->scrollCallback(xOffset, yOffset);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
		inputHandler->keyCallback(key, action, mods);
		std::cout << key << " / " << scancode << " / " << action << " / " << mods << std::endl;
	}

	void mouseButton_callback(GLFWwindow* window, int button, int action, int mods)
	{
		InputHandler* inputHandler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
		inputHandler->mouseCallback(button, action);
	}
}

void InputHandler::initialize(InputManager* inInputManager, WindowManager* inWindowManager)
{
	inputManager = inInputManager;
	windowManager = inWindowManager;
	if (!inputManager || !windowManager)
	{
		std::cout << "ERROR : no inputManager/window manager" << std::endl;
		return;
	}
	
	GLFWwindow* window = windowManager->getWindow();
	glfwSetWindowUserPointer(window, this);

	// window events
	glfwSetFramebufferSizeCallback(window, WindowEvents::framebuffer_size_callback);

	// input events
	glfwSetKeyCallback(window, InputCallbacks::key_callback);
	glfwSetMouseButtonCallback(window, InputCallbacks::mouseButton_callback);
	glfwSetScrollCallback(window, InputCallbacks::scroll_callback);
	glfwSetCursorPosCallback(window, InputCallbacks::cursorPos_callback);
}


void InputHandler::mouseCallback(int inButton, int inState)
{
	inputManager->mouseInputs.push_back({ inButton, inState });
}

void InputHandler::keyCallback(int inKey, int inState, int inMods)
{
	inputManager->keyInputs.push_back({ inKey, inState, inMods });
}

void InputHandler::cursorPosCallback(double inX, double inY)
{
	inputManager->cursorPosInputs.push_back({ inX, inY });
}

void InputHandler::scrollCallback(double inX, double inY)
{
	inputManager->scrollInputs.push_back({ inX, inY });
}
