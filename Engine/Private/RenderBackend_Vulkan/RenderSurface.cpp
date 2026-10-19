#include "RenderBackend_Vulkan/RenderSurface.h"

#include "RenderBackend/RenderBackendDef.h"
#include "Platform/PlatformDef.h"

#ifdef PLATFORM_GLFW
	#include "RenderBackend_Vulkan/RenderSurface_GLFW.inl"
#else
#endif