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
	render::getRenderBackend()->deleteTexture(handle);
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
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cout << "ERROR: Failed to load texture: " << path << std::endl;
		return;
	}

	ETextureFormat textureFormat;
	switch (nrChannels)
	{
	case 3:
		textureFormat = ETextureFormat::RGB;
		break;
	case 4:
		textureFormat = ETextureFormat::RGBA;
		break;
	default:
		ensure(false);
		break;
	}

	handle = render::getRenderBackend()->createTexture(data, width, height, textureFormat);

	stbi_image_free(data);
}