#pragma once

#include <iostream>

#include "glad/glad.h"

class Texture
{
public:
	Texture(const char* texturePath, GLenum textureFormat)
	{
		textureID = loadTexture(texturePath, textureFormat);
	}

	~Texture()
	{
		//glDeleteTextures(1, textureID);
	}

	GLuint GetTextureID() const
	{
		return textureID;
	}

private:
	GLuint textureID;

	static GLuint loadTexture(const char* texturePath, GLenum textureFormat);
};
