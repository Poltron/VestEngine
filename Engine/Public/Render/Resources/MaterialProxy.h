#pragma once

#include "RenderBackend/RenderBackendDef.h"

#ifdef RENDER_OPENGL
	#include "RenderBackend_OpenGL/OpenGLMaterialProxy.h"
	using MaterialProxy = OpenGLMaterialProxy;
#elif defined(RENDER_VULKAN)
	#include "RenderBackend_Vulkan/VulkanMaterialProxy.h"
	using MaterialProxy = VulkanMaterialProxy;
#else
	#include "Core/Ensure.h"
	ensure(false);
#endif