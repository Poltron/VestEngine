#pragma once

class InputManager;
class WindowManager;

// note : feels bad to have all this public but no choice when glfw input callbacks can't be class members ?
struct InputHandler
{
	void initialize(InputManager* inInputManager, WindowManager* inWindowManager);

	InputManager* inputManager;

	// todo : implement some window manager event forwarding
	WindowManager* windowManager;

	void mouseCallback(int inButton, int inState);
	void keyCallback(int inKey, int inState, int inMods);
	void cursorPosCallback(double inX, double inY);
	void scrollCallback(double inX, double inY);
};
