#pragma once

#include <vulkan/vulkan.h>

#include "Core/Ensure.h"

class VulkanBuffer
{
	VkDevice device = VK_NULL_HANDLE;

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size;

	void* mappedMemory = nullptr;
	uint32_t offset;

public:
	VkDevice getDevice() const { return device; }
	VkBuffer getBuffer() const { return buffer; }
	VkDeviceMemory getMemory() const { return memory; }
	VkDeviceSize getSize() const { return size; }
	void* getMappedMemory() const { return mappedMemory; }
	bool isMapped() const { return mappedMemory != nullptr; }

	VulkanBuffer(VkDevice inDevice, VkBuffer inBuffer, VkDeviceMemory inMemory, VkDeviceSize inSize)
		: device(inDevice), buffer(inBuffer), memory(inMemory), size(inSize), mappedMemory(nullptr), offset(0)
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

	BufferHandle push_back(void* inData, uint32_t inSize, uint32_t inOffsetAlignment)
	{
		ensure(isMapped());
		ensure(getSize() >= offset + inSize);

		BufferHandle handle = reserve(inSize, inOffsetAlignment);
		fill(inData, handle.size, handle.offset);
		return handle;
	}

	BufferHandle reserve(uint32_t inSize, uint32_t inOffsetAlignment)
	{
		ensure(getSize() >= offset + inSize);

		BufferHandle handle(GPURESOURCE_INVALID_ID, offset, inSize);
		offset += getPaddedSize(inSize, inOffsetAlignment);
		return handle;
	}

	void reset()
	{
		offset = 0;
	}

private:
	uint32_t getPaddedSize(uint32_t inSize, uint32_t inAlignment)
	{
		ensure(inAlignment > 0);
		return (inSize + inAlignment - 1) & ~(inAlignment - 1);
	}
};