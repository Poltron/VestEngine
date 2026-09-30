#pragma once

#include <functional>

#include "glm/glm.hpp"

class WindowManager
{
protected:
	glm::vec2 size;
	void* window;

public:
	void* getWindow() { return window; }
	const void* getWindow() const { return window; }

public:
	virtual void swapBuffers() = 0;
	virtual void resize(int inWidth, int inHeight) = 0;

	const glm::vec2& getSize() const { return size; }

	using WindowCloseRequestCallback = std::function<void()>;
	void registerWindowCloseRequestCallback(WindowCloseRequestCallback inScrollback);

protected:
	WindowCloseRequestCallback windowCloseRequestCallback;
};