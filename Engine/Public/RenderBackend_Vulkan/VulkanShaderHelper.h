#pragma once

#include <vulkan/vulkan.h>

#include "Render/ShaderStructs.h"

namespace shader
{
	VkShaderStageFlags getShaderStageFlags(ShaderStageFlags inShaderStageFlags);

	void extractShaderParameters(const std::vector<char>& inBytes, ShaderParameters& outShaderParameters);

	bool createVkShaderModule(VkDevice inDevice, const std::vector<char>& inCode, VkShaderModule& outShaderModule);
}