#include "RenderBackend/RenderBackend.h"

#ifdef RENDER_OPENGL
#include "RenderBackend_OpenGL/RenderBackend_OpenGL.inl"
#else
	#ifdef RENDER_VULKAN
	#include "RenderBackend_Vulkan/RenderBackend_Vulkan.inl"
	#else
	#error Graphic API not supported
	#endif
#endif