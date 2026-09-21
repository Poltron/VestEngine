#pragma once

#include <functional>

class WindowManager
{
public:
	void* getWindow() { return window; }
	const void* getWindow() const { return window; }
protected:
	void* window;

public:
	virtual void swapBuffers() = 0;
	virtual void resize(int inWidth, int inHeight) = 0;

	using WindowCloseRequestCallback = std::function<void()>;
	void registerWindowCloseRequestCallback(WindowCloseRequestCallback inScrollback);

protected:
	WindowCloseRequestCallback windowCloseRequestCallback;
};