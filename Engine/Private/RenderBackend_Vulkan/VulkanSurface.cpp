#include "RenderBackend_Vulkan/VulkanSurface.h"

#include "RenderBackend/RenderBackendDef.h"
#include "Platform/PlatformDef.h"

#ifdef PLATFORM_GLFW
	#include "RenderBackend_Vulkan/VulkanSurface_GLFW.inl"
#else
#endif