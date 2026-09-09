#pragma once

class InputManager;
class WindowManager;

namespace platform
{
	void initialize();
	void shutdown();

	double getTime();

	InputManager& getInputManager();
	WindowManager& getWindowManager();
}

