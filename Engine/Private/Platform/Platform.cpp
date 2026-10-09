#include "Platform/Platform.h"

#ifdef PLATFORM_GLFW
#include "Platform/GLFW/GLFWPlatform.inl"
#else
#error Platform not supported
#endif