#include "Render/Resources/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Core/Ensure.h"
#include "RenderBackend/RenderBackend.h"

Texture::Texture(std::string inPath, std::string inType)
{
	path = std::move(inPath);
	type = std::move(inType);

	loadTexture();
}

Texture::~Texture()
{
	if (handle.isValid())
	{
		render::getRenderBackend()->deleteTexture(handle);
	}
}

Texture::Texture(Texture&& inOther) noexcept
	: handle(inOther.handle), type(inOther.type), path(inOther.path)
{
	inOther.handle = {};
	inOther.type.clear();
	inOther.path.clear();
}

Texture& Texture::operator=(Texture&& inOther) noexcept
{
	if (this != &inOther)
	{
		handle = inOther.getHandle();
		type = inOther.type;
		path = inOther.path;

		inOther.handle = {};
		inOther.type.clear();
		inOther.path.clear();
	}
	return *this;
}

void Texture::loadTexture()
{
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	if (!data)
	{
		std::cout << "ERROR: Failed to load texture: " << path << std::endl;
		return;
	}

	handle = render::getRenderBackend()->createTexture(data, width, height);

	stbi_image_free(data);
}