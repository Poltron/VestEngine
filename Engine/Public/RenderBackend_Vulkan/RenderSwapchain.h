#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RenderDevice;

class RenderSwapchain
{
	VkDevice device;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

public:
	VkSwapchainKHR get() const { return swapchain; };
	VkFormat getImageFormat() const { return imageFormat; };
	VkExtent2D getExtent() const { return extent; };
	const std::vector<VkImage> getImages() const { return images; }
	const std::vector<VkImageView>& getImageViews() const { return imageViews; }

	bool initialize(const RenderDevice& inDevice, VkSurfaceKHR inSurface);
	void shutdown();

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& inAvailableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& inAvailablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& inCapabilities);

	bool createImageViews(VkSurfaceKHR inSurface);
};