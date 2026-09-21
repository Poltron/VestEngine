#include "Platform/Platform.h"

// todo: have a define or smth
#if true
#include "Platform/GLFW/GLFWPlatform.inl"
#else
#error Platform not supported
#endif