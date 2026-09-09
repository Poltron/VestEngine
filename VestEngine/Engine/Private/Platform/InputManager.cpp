#include "Platform/InputManager.h"

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

void InputManager::initialize()
{
	registerKeyCallback(input::EKey::ESCAPE
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			platform::getWindowManager().closeWindow();
		});
}

void InputManager::processInput(double inDeltaTime)
{
	pollEvents();

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

	keyInputs.clear();
	mouseInputs.clear();
	cursorPosInputs.clear();
	scrollInputs.clear();
}

void InputManager::registerKeyCallback(input::EKey key, KeyCallback inCallback)
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

void InputManager::registerMouseCallback(input::EMouseButton button, MouseCallback inCallback)
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