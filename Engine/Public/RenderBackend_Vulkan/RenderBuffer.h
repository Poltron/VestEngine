#pragma once

#include <vulkan/vulkan.h>

#include "Core/Ensure.h"

class RenderBuffer
{
	VkDevice device = VK_NULL_HANDLE;

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size;

	void* mappedMemory = nullptr;

public:
	VkDevice getDevice() const { return device; }
	VkBuffer getBuffer() const { return buffer; }
	VkDeviceMemory getMemory() const { return memory; }
	VkDeviceSize getSize() const { return size; }
	void* getMappedMemory() const { return mappedMemory; }
	bool isMapped() const { return mappedMemory != nullptr; }

	RenderBuffer(VkDevice inDevice, VkBuffer inBuffer, VkDeviceMemory inMemory, VkDeviceSize inSize)
		: device(inDevice), buffer(inBuffer), memory(inMemory), size(inSize), mappedMemory(nullptr)
	{}

	void fill(void* inData, size_t inDataSize, uint32_t inOffset = 0)
	{
		ensure(isMapped());

		memcpy((uint8_t*)mappedMemory + inOffset, inData, inDataSize);
	}

	void mapAndFill(void* inData, size_t inDataSize, uint32_t inOffset = 0)
	{
		ensure(!isMapped());

		map(inDataSize);
		memcpy((uint8_t*)mappedMemory + inOffset, inData, inDataSize);
		unmap();
	}

	void map(VkDeviceSize inMappedSize)
	{
		ensure(size >= inMappedSize);
		vkMapMemory(device, memory, 0, inMappedSize, 0, &mappedMemory);
	}

	void unmap()
	{
		vkUnmapMemory(device, memory);
		mappedMemory = nullptr;
	}
};