#pragma once

#include "Core/Ensure.h"

enum class DescriptorType : uint8_t
{
	UNIFORM_BUFFER,
	TEXTURE_SAMPLER
};

inline VkDescriptorType getDescriptorType(DescriptorType inDescriptorType)
{
	switch (inDescriptorType)
	{
	case DescriptorType::UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DescriptorType::TEXTURE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	}

	ensure(false);
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}