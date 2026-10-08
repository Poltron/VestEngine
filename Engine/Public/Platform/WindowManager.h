#pragma once

#include <functional>

#include "glm/glm.hpp"

class WindowManager
{
	friend class InputHandler;

protected:
	glm::vec2 size;
	void* window;

public:
	void shutdown();
	void* createWindow(unsigned int width, unsigned int height);
	void closeWindow();

	void* getWindow() { return window; }
	const void* getWindow() const { return window; }

public:
	void swapBuffers();
	void resize(unsigned int inWidth, unsigned int inHeight);

	const glm::vec2& getSize() const { return size; }

	using WindowCloseRequestCallback = std::function<void()>;
	void registerWindowCloseRequestCallback(WindowCloseRequestCallback inScrollback);

private:
	WindowCloseRequestCallback windowCloseRequestCallback;

	void receivedCloseWindowRequest()
	{
		windowCloseRequestCallback();
	}
};