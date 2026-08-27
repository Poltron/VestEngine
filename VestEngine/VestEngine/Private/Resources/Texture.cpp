#include "Resources/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Utils/Assert.h"

Texture::Texture(const char* inPath, const char* inType)
	: path(inPath), type(inType)
{
	path = inPath;
	type = inType;

	loadTexture();
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureID);
}

Texture::Texture(Texture&& inOther) noexcept
	: textureID(inOther.textureID), type(inOther.type), path(inOther.path)
{
	inOther.textureID = 0;
	inOther.type.clear();
	inOther.path.clear();
}

Texture& Texture::operator=(Texture&& inOther) noexcept
{
	if (this != &inOther)
	{
		textureID = inOther.textureID;
		type = inOther.type;
		path = inOther.path;

		inOther.textureID = 0;
		inOther.type.clear();
		inOther.path.clear();
	}
	return *this;
}

void Texture::loadTexture()
{
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cout << "Failed to load texture: " << path << std::endl;
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
			SOFT_ASSERT(false, "Texture format not handled");
			break;

	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}