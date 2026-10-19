#pragma once

#include <vulkan/vulkan.h>

class RenderBuffer
{
	VkDevice device = VK_NULL_HANDLE;

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;

public:
	VkDevice getDevice() const { return device; }
	VkBuffer getBuffer() const { return buffer; }
	VkDeviceMemory getMemory() const { return memory; }

	RenderBuffer(VkDevice inDevice, VkBuffer inBuffer, VkDeviceMemory inMemory)
		: device(inDevice), buffer(inBuffer), memory(inMemory)
	{}
};