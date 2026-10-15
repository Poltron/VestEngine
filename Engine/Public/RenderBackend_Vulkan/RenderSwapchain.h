#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RenderDevice;

class RenderSwapchain
{
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> images;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;

public:
	VkSwapchainKHR get() { return swapchain; };
	std::vector<VkImage> getImages() { return images; }
	VkFormat getImageFormat() { return imageFormat; };
	VkExtent2D getExtent() { return extent; };
	std::vector<VkImageView>& getImageViews() { return imageViews; }
	std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }

	bool initialize(RenderDevice& inDevice, VkSurfaceKHR inSurface);
	void shutdown(RenderDevice& inDevice);

	bool createImageViews(VkPhysicalDevice inPhysicalDevice, VkDevice inDevice, VkSurfaceKHR inSurface);
	bool createFramebuffers(VkDevice inDevice, VkRenderPass inRenderPass);

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	static bool createImageView(VkDevice inDevice, VkImage inImage, VkFormat inFormat, VkImageView& outImageView);

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& inAvailableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& inAvailablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& inCapabilities);
};