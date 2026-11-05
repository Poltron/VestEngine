#pragma once

#include "RenderBackend/RenderBackendDef.h"

#ifdef RENDER_OPENGL
	#include "RenderBackend_OpenGL/OpenGLMeshProxy.h"
	using MeshProxy = OpenGLMeshProxy;
#elif defined(RENDER_VULKAN)
	#include "RenderBackend_Vulkan/VulkanMeshProxy.h"
	using MeshProxy = VulkanMeshProxy;
#else
	#include "Core/Ensure.h"
	ensure(false);
#endif