#include "RenderBackend_Vulkan/RenderImage.h"

#include <iostream>

#include "RenderBackend_Vulkan/RenderDevice.h"

bool RenderImage::createImage(VkDevice inDevice
	, VkPhysicalDevice inPhysicalDevice
	, uint32_t inWidth
	, uint32_t inHeight
	, VkFormat inFormat
	, VkImageTiling inTiling
	, VkImageUsageFlags inUsage
	, const std::vector<uint32_t>& inQueueIndices
	, VkMemoryPropertyFlags inProperties
	, VkImage& outTextureImage
	, VkDeviceMemory& outTextureMemory)
{

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = (uint32_t)inWidth;
	imageInfo.extent.height = (uint32_t)inHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = inFormat;
	imageInfo.tiling = inTiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = inUsage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (inQueueIndices.size() >= 2)
	{
		imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		imageInfo.queueFamilyIndexCount = (uint32_t)inQueueIndices.size();
		imageInfo.pQueueFamilyIndices = inQueueIndices.data();
	}
	else
	{
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VkResult result = vkCreateImage(inDevice, &imageInfo, nullptr, &outTextureImage);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateImage failed" << std::endl;
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(inDevice, outTextureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t memoryType;
	RenderDevice::findMemoryType(inPhysicalDevice, memRequirements.memoryTypeBits, inProperties, memoryType);
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(inDevice, &allocInfo, nullptr, &outTextureMemory);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateMemory failed" << std::endl;
		return false;
	}

	vkBindImageMemory(inDevice, outTextureImage, outTextureMemory, 0);
	return true;
}

bool RenderImage::createImageView(VkDevice inDevice
	, VkImage inTextureImage
	, VkFormat inFormat
	, VkImageAspectFlags inAspectFlags
	, VkImageView& outImageView)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = inTextureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = inFormat;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = inAspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(inDevice, &viewInfo, nullptr, &outImageView);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateImageView failed" << std::endl;
		return false;
	}
	return true;
}