#include "Platform/InputManager.h"

#include "tracy/Tracy.hpp"

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

bool InputManager::initialize()
{
	return true;
}

void InputManager::shutdown()
{
	keyCallbacks.clear();
	mouseCallbacks.clear();
	cursorPosCallbacks.clear();
	scrollCallbacks.clear();
}

void InputManager::processInput(double inDeltaTime)
{
	ZoneScoped;

	pollEvents();

	for (auto& keyInput : keyInputs)
	{
		if (auto existingPair = keyCallbacks.find(keyInput.key); existingPair != keyCallbacks.end())
		{
			for (auto& keyCallback : existingPair->second)
			{
				if (!keyCallback)
				{
					continue;
				}

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
				if (!mouseCallback)
				{
					continue;
				}

				mouseCallback(mouseInput.state, inDeltaTime);
			}
		}
	}

	for (auto& cursorPosInput : cursorPosInputs)
	{
		for (auto& cursorPosCallback : cursorPosCallbacks)
		{
			if (!cursorPosCallback)
			{
				continue;
			}

			cursorPosCallback(cursorPosInput.x, cursorPosInput.y, inDeltaTime);
		}
	}

	for (auto& scrollInput : scrollInputs)
	{
		for (auto& scrollCallback : scrollCallbacks)
		{
			if (!scrollCallback)
			{
				continue;
			}

			scrollCallback(scrollInput.x, scrollInput.y, inDeltaTime);
		}
	}

	keyInputs.clear();
	mouseInputs.clear();
	cursorPosInputs.clear();
	scrollInputs.clear();
}

input::InputCallbackHandle InputManager::registerKeyCallback(input::EKey inKey, KeyCallback inCallback)
{
	auto specifiedKeyIt = keyCallbacks.find(inKey);
	if (specifiedKeyIt == keyCallbacks.end())
	{
		keyCallbacks.insert({ inKey, { inCallback } });
		return input::InputCallbackHandle(0);
	}

	std::vector<KeyCallback>& myKeyCallbacks = specifiedKeyIt->second;
	for (size_t i = 0; i < myKeyCallbacks.size(); ++i)
	{
		if (!myKeyCallbacks[i])
		{
			myKeyCallbacks[i] = inCallback;
			return input::InputCallbackHandle(i);
		}
	}

	myKeyCallbacks.push_back(inCallback);
	return input::InputCallbackHandle(myKeyCallbacks.size() - 1);
}

input::InputCallbackHandle InputManager::registerMouseCallback(input::EMouseButton inButton, MouseCallback inCallback)
{
	auto specifiedButtonIt = mouseCallbacks.find(inButton);
	if (specifiedButtonIt == mouseCallbacks.end())
	{
		mouseCallbacks.insert({ inButton, { inCallback } });
		return input::InputCallbackHandle(0);
	}

	std::vector<MouseCallback>& myMouseCallbacks = specifiedButtonIt->second;
	for (size_t i = 0; i < myMouseCallbacks.size(); ++i)
	{
		if (!myMouseCallbacks[i])
		{
			myMouseCallbacks[i] = inCallback;
			return input::InputCallbackHandle(i);
		}
	}

	myMouseCallbacks.push_back(inCallback);
	return input::InputCallbackHandle(myMouseCallbacks.size() - 1);
}

input::InputCallbackHandle InputManager::registerCursorPosCallback(CursorPosCallback inCallback)
{
	for (size_t i = 0; i < cursorPosCallbacks.size(); ++i)
	{
		if (!cursorPosCallbacks[i])
		{
			cursorPosCallbacks[i] = inCallback;
			return input::InputCallbackHandle(i);
		}
	}

	cursorPosCallbacks.push_back(inCallback);
	return input::InputCallbackHandle(cursorPosCallbacks.size() - 1);
}

input::InputCallbackHandle InputManager::registerScrollCallback(ScrollCallback inCallback)
{
	for (size_t i = 0; i < scrollCallbacks.size(); ++i)
	{
		if (!scrollCallbacks[i])
		{
			scrollCallbacks[i] = inCallback;
			return input::InputCallbackHandle(i);
		}
	}

	scrollCallbacks.push_back(inCallback);
	return input::InputCallbackHandle(scrollCallbacks.size() - 1);
}

void InputManager::unregisterKeyCallback(input::EKey inKey, input::InputCallbackHandle inHandle)
{
	if (!inHandle.isValid())
	{
		return;
	}

	auto keyIt = keyCallbacks.find(inKey);
	if (keyIt == keyCallbacks.end()
		|| keyIt->second.size() <= inHandle.id)
	{
		return;
	}
	
	keyIt->second.at(inHandle.id) = {};
}

void InputManager::unregisterMouseCallback(input::EMouseButton inButton, input::InputCallbackHandle inHandle)
{
	if (!inHandle.isValid())
	{
		return;
	}

	auto keyIt = mouseCallbacks.find(inButton);
	if (keyIt == mouseCallbacks.end()
		|| keyIt->second.size() <= inHandle.id)
	{
		return;
	}

	keyIt->second.at(inHandle.id) = {};
}

void InputManager::unregisterCursorCallback(input::InputCallbackHandle inHandle)
{
	if (!inHandle.isValid()
		|| cursorPosCallbacks.size() <= inHandle.id)
	{
		return;
	}

	cursorPosCallbacks[inHandle.id] = {};
}

void InputManager::unregisterScrollCallback(input::InputCallbackHandle inHandle)
{
	if (!inHandle.isValid()
		|| scrollCallbacks.size() <= inHandle.id)
	{
		return;
	}

	scrollCallbacks[inHandle.id] = {};
}