#pragma once

#include <vulkan/vulkan.h>

#include "Render/ShaderStructs.h"

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