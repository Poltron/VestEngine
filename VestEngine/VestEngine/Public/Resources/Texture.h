#pragma once

#include <iostream>

#include "glad/glad.h"

class Texture
{
public:
	~Texture();

	void loadTexture(const char* inTexturePath, const std::string& inType);

	GLuint getTextureID() const { return textureID; }
	const std::string& getType() const { return type; }
	const std::string& getPath() const { return path; }

private:
	GLuint textureID;
	std::string type;
	std::string path;
};
