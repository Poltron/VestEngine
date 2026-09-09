#pragma once

class InputManager;
class WindowManager;

namespace platform
{
	void initialize();
	void shutdown();

	InputManager& getInputManager();
	WindowManager& getWindowManager();
}

