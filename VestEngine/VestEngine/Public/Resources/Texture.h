#pragma once

#include <iostream>

#include "glad/glad.h"

class Texture
{
public:
	Texture() 
	{ }

	~Texture()
	{
		glDeleteTextures(1, &textureID);
	}

	void loadTexture(const char* inTexturePath, GLenum inTextureFormat, const std::string& inType);

	GLuint GetTextureID() const { return textureID; }

private:
	GLuint textureID;
	std::string type;
};
