#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RenderImage
{
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

public:
	static bool createImage(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties, VkImage& outTextureImage, VkDeviceMemory& outTextureMemory);
	static bool createImageView(VkDevice inDevice, VkImage inTextureImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags, VkImageView& outImageView);
};