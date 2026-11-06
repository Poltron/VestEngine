#pragma once

#include <string>
#include <vector>

#include "Render/GPUHandle.h"
#include "Render/ShaderStructs.h"
#include "RenderBackend/RenderBackendDef.h"

class Texture
{
	std::string type;
	std::string path;

	int width;
	int height;
	int channelCount;
	std::vector<uint8_t> data;
	EDataFormat format;
	GPUProxyHandle handle;

public:
	const std::string& getType() const { return type; }
	const std::string& getPath() const { return path; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	int getChannelCount() const { return channelCount; }
	const uint8_t* getData() const { return data.data(); }
	GPUProxyHandle getProxyHandle() const { return handle; }

	Texture() = delete;
	Texture(std::string inPath, std::string inType);
	~Texture();

	Texture(const Texture& inOther) = delete;
	Texture& operator=(const Texture& inOther) = delete;

	Texture(Texture&& inOther) noexcept;
	Texture& operator=(Texture&& inOther) noexcept;
};