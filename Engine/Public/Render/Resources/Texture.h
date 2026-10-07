#pragma once

#include <iostream>

#include "Render/GraphicResourceHandle.h"

class Texture
{
public:
	Texture() = delete;
	Texture(std::string inPath, std::string inType);
	~Texture();

	Texture(const Texture& inOther) = delete;
	Texture& operator=(const Texture& inOther) = delete;

	Texture(Texture&& inOther) noexcept;
	Texture& operator=(Texture&& inOther) noexcept;

	GPUHandle getHandle() const { return handle; }
	const std::string& getType() const { return type; }
	const std::string& getPath() const { return path; }

private:
	void loadTexture();

	GPUHandle handle;
	std::string type;
	std::string path;
};
