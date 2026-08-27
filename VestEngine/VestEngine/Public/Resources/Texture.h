#pragma once

#include <iostream>

#include "glad/glad.h"

class Texture
{
public:
	Texture() = delete;
	Texture(const char* inPath, const char* inType);
	~Texture();

	Texture(const Texture& inOther) = delete;
	Texture& operator=(const Texture& inOther) = delete;

	Texture(Texture&& inOther) noexcept;
	Texture& operator=(Texture&& inOther) noexcept;

	GLuint getTextureID() const { return textureID; }
	const std::string& getType() const { return type; }
	const std::string& getPath() const { return path; }

private:
	void loadTexture();

	GLuint textureID;
	std::string type;
	std::string path;
};
