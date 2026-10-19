#include "RenderBackend_Vulkan/RenderImage.h"

#include <iostream>

#include "RenderBackend_Vulkan/RenderDevice.h"

VkImageView RenderImage::createImageView(VkDevice inDevice
	, VkImage inTextureImage
	, VkFormat inFormat
	, VkImageAspectFlags inAspectFlags)
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

	VkImageView view;
	VkResult result = vkCreateImageView(inDevice, &viewInfo, nullptr, &view);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateImageView failed" << std::endl;
	}
	return view;
}