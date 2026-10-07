#include "Render/Resources/Texture.h"

#include "glad/glad.h"

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

	GLenum internalFormat = GL_NONE;
	switch (nrChannels)
	{
		case 3:
			internalFormat = GL_RGB;
			break;
		case 4:
			internalFormat = GL_RGBA;
			break;
		default:
			ensure(false);
			break;

	}

	glGenTextures(1, &handle.id);
	glBindTexture(GL_TEXTURE_2D, handle.id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	render::getRenderBackend()->createTexture(data);

	stbi_image_free(data);
}