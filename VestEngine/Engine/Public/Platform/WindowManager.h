#pragma once

class WindowManager
{
public:
	void* getWindow() { return window; }
	const void* getWindow() const { return window; }

	virtual void* createWindow(int width, int height) = 0;
	virtual void destroyWindow() = 0;

	virtual bool shouldCloseWindow() const = 0;

	virtual void swapBuffers() = 0;
	virtual void resize(int inWidth, int inHeight) = 0;

protected:
	void* window;
};