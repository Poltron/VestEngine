#include "Core/Resources/Texture.h"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Render/Renderer.h"

Texture::Texture(std::string inPath, std::string inType)
	: handle(GPUProxyHandle::invalid)
{
	path = std::move(inPath);
	type = std::move(inType);

	stbi_set_flip_vertically_on_load(true);

	uint8_t* tmpData = stbi_load(path.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
	if (!tmpData)
	{
		std::cout << "ERROR: Failed to load texture: " << path << std::endl;
		return;
	}

	// note: STBI_rgb_alpha forces 4 channels
	channelCount = 4;

	size_t size = width * height * channelCount;
	data.resize(size);
	memcpy(data.data(), tmpData, size);

	handle = render::getRenderer()->createTextureProxy(this);

	stbi_image_free(tmpData);
}

Texture::~Texture()
{
	if (handle.isValid())
	{
		render::getRenderer()->destroyTextureProxy(handle);
	}
}

Texture::Texture(Texture&& inOther) noexcept
	: handle(inOther.handle), type(inOther.type), path(inOther.path)
{
	inOther.handle = GPUProxyHandle::invalid;
	inOther.type.clear();
	inOther.path.clear();
}

Texture& Texture::operator=(Texture&& inOther) noexcept
{
	if (this != &inOther)
	{
		handle = inOther.getProxyHandle();
		type = inOther.type;
		path = inOther.path;

		inOther.handle = GPUProxyHandle::invalid;
		inOther.type.clear();
		inOther.path.clear();
	}
	return *this;
}