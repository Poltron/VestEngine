#pragma once

#define PLATFORM_GLFW

class InputManager;
class WindowManager;

class InputHandler;

namespace platform
{
	bool initialize();
	void shutdown();

	double getTime();

	InputManager& getInputManager();
	WindowManager& getWindowManager();
}

