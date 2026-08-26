#include "Resources/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::~Texture()
{
	glDeleteTextures(1, &textureID);
}

void Texture::loadTexture(const char* inTexturePath, const std::string& inType)
{
	type = inType;
	path = inTexturePath;

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(inTexturePath, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cout << "Failed to load texture: " << inTexturePath << std::endl;
	}
	
	GLenum internalFormat = GL_RGBA;
	if (nrChannels == 3)
	{
		internalFormat = GL_RGB;
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