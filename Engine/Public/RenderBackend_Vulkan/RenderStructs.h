#pragma once

#include "Core/Ensure.h"

enum EShaderStageFlagBits : uint8_t
{
	NONE = 0x00,
	VERTEX = 0x01,
	FRAGMENT = 0x02
};

struct ShaderStageInfo
{
	std::string relativePath;
	EShaderStageFlagBits stage;
};
typedef uint8_t ShaderStageFlags;

inline VkShaderStageFlags getShaderStageFlags(ShaderStageFlags inShaderStageFlags)
{
	VkShaderStageFlags shader = 0;
	if (inShaderStageFlags & EShaderStageFlagBits::FRAGMENT)
	{
		shader = shader | VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	if (inShaderStageFlags & EShaderStageFlagBits::VERTEX)
	{
		shader = shader | VK_SHADER_STAGE_VERTEX_BIT;
	}
	return shader;
}