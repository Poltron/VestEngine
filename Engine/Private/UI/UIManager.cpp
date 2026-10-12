#include "UI/UIManager.h"

#include "RenderBackend/RenderBackend.h"

#ifdef RENDER_OPENGL
#include "UI/GLFW_OpenGL3/GLFW_OpenGL_UIManager.inl"
#else
	#ifdef RENDER_VULKAN
	#include "UI/GLFW_Vulkan/GLFW_Vulkan_UIManager.inl"
	#else
	#error Platform not supported
	#endif
#endif