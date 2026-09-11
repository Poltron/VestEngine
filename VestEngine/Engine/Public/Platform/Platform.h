#pragma once

class InputManager;
class WindowManager;

namespace platform
{
	bool initialize();
	void shutdown();

	double getTime();

	InputManager& getInputManager();
	WindowManager& getWindowManager();
}

