#pragma once

struct GLFWwindow;

class WindowManager
{
public:
	GLFWwindow* getWindow() { return window; }
	const GLFWwindow* getWindow() const { return window; }

	GLFWwindow* createWindow(int width, int height);
	void destroyWindow();

	bool shouldCloseWindow() const;

private:
	GLFWwindow* window;

	friend struct InputHandler;
};