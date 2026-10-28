#pragma once

#include <vulkan/vulkan.h>

class RenderImage
{
	VkDevice device = VK_NULL_HANDLE;

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;

public:
	VkDevice getDevice() const { return device; }
	VkImage getImage() const { return image; }
	VkDeviceMemory getMemory() const { return memory; }
	VkImageView getView() const { return view; }

	RenderImage(VkDevice inDevice, VkImage inImage, VkDeviceMemory inMemory)
		: device(inDevice), image(inImage), memory(inMemory)
	{ }

	void setTextureView(VkImageView inView)
	{
		view = inView;
	}

	static VkImageView createImageView(VkDevice inDevice, VkImage inTextureImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags);
};