#include "Platform/WindowManager.h"

void WindowManager::registerWindowCloseRequestCallback(WindowCloseRequestCallback inCallback)
{
	windowCloseRequestCallback = inCallback;
}