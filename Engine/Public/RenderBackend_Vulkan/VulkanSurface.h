#pragma once

#include <vulkan/vulkan.h>

class VulkanSurface
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	
public:
	VkSurfaceKHR getSurface() const { return surface; }
	
	bool create(VkInstance inInstance);
	void destroy(VkInstance inInstance);
};