#include "Managers/InputManager.h"

#include "GLFW/glfw3.h"

#include <iostream>

#include "Managers/InputHandler.h"
#include "Managers/WindowManager.h"

void InputManager::initialize(GLFWwindow* inWindow)
{
	glfwSetInputMode(inWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::processInput(GLFWwindow* inWindow, double inDeltaTime)
{
	glfwPollEvents();

	for (auto& keyInput : keyInputs)
	{
		if (auto existingPair = keyCallbacks.find(keyInput.key); existingPair != keyCallbacks.end())
		{
			for (auto& keyCallback : existingPair->second)
			{
				keyCallback(keyInput.state, keyInput.mods, inDeltaTime);
			}
		}
	}

	for (auto& mouseInput : mouseInputs)
	{
		if (auto existingPair = mouseCallbacks.find(mouseInput.button); existingPair != mouseCallbacks.end())
		{
			for (auto& mouseCallback : existingPair->second)
			{
				mouseCallback(mouseInput.state, inDeltaTime);
			}
		}
	}

	for (auto& cursorPosInput : cursorPosInputs)
	{
		for (auto& cursorPosCallback : cursorPosCallbacks)
		{
			cursorPosCallback(cursorPosInput.x, cursorPosInput.y, inDeltaTime);
		}
	}

	for (auto& scrollInput : scrollInputs)
	{
		for (auto& scrollCallback : scrollCallbacks)
		{
			scrollCallback(scrollInput.x, scrollInput.y, inDeltaTime);
		}
	}


	if (glfwGetKey(inWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(inWindow, true);
	}

	//if (glfwGetKey(inWindow, GLFW_KEY_T) == GLFW_PRESS)
	//{
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//}
	//if (glfwGetKey(inWindow, GLFW_KEY_Y) == GLFW_PRESS)
	//{
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//}
}

void InputManager::registerKeyCallback(int key, KeyCallback inCallback)
{
	// note : syntaxe des enfers, contains() seulement en C++20 ???
	if (auto existingPair = keyCallbacks.find(key); existingPair != keyCallbacks.end())
	{
		existingPair->second.push_back(inCallback);
	}
	else
	{
		keyCallbacks.insert({ key, { inCallback } });
	}
}

void InputManager::registerMouseCallback(int button, MouseCallback inCallback)
{
	// note : syntaxe des enfers, contains() seulement en C++20 ???
	if (auto existingPair = mouseCallbacks.find(button); existingPair != mouseCallbacks.end())
	{
		existingPair->second.push_back(inCallback);
	}
	else
	{
		mouseCallbacks.insert({ button, { inCallback } });
	}
}

void InputManager::registerCursorPosCallback(CursorPosCallback inCallback)
{
	cursorPosCallbacks.push_back(inCallback);
}

void InputManager::registerScrollCallback(ScrollCallback inScrollback)
{
	scrollCallbacks.push_back(inScrollback);
}