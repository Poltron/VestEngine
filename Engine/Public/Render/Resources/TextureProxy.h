#pragma once

#include <string>

#include "Render/GPUHandle.h"
#include "RenderBackend/RenderBackendDef.h"

#ifdef RENDER_OPENGL
// todo
using TextureView = GLuint;
#elif defined(RENDER_VULKAN)
#include <vulkan/vulkan.h>
using TextureView = VkImageView;
#endif

class Texture;

class TextureProxy
{
	GPUAllocHandle handle;

public:
	GPUAllocHandle getHandle() const { return handle; }

	TextureProxy() = delete;
	TextureProxy(const Texture* inTexture);
	~TextureProxy();

	TextureProxy(const TextureProxy& inOther) = delete;
	TextureProxy& operator=(const TextureProxy& inOther) = delete;

	TextureProxy(TextureProxy&& inOther) noexcept;
	TextureProxy& operator=(TextureProxy&& inOther) noexcept;
};