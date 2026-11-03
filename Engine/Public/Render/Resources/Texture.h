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

class Texture
{
public:
	Texture() = delete;
	Texture(std::string inPath, std::string inType);
	~Texture();

	Texture(const Texture& inOther) = delete;
	Texture& operator=(const Texture& inOther) = delete;

	Texture(Texture&& inOther) noexcept;
	Texture& operator=(Texture&& inOther) noexcept;

	GPUHandle getHandle() const { return handle; }
	const std::string& getType() const { return type; }
	const std::string& getPath() const { return path; }

private:
	void loadTexture();
	
	GPUHandle handle;
	std::string type;
	std::string path;
};