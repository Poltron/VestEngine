#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RenderDevice;

class RenderSwapchain
{
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

public:
	VkSwapchainKHR get() { return swapchain; };
	VkFormat getImageFormat() { return imageFormat; };
	VkExtent2D getExtent() { return extent; };
	std::vector<VkImage> getImages() { return images; }
	std::vector<VkImageView>& getImageViews() { return imageViews; }
	std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }

	bool initialize(RenderDevice& inDevice, VkSurfaceKHR inSurface);
	void shutdown(RenderDevice& inDevice);

	bool createImageViews(VkPhysicalDevice inPhysicalDevice, VkDevice inDevice, VkSurfaceKHR inSurface);
	bool createDepthResources(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkCommandPool inCommandPool, VkQueue inQueue);
	bool createFramebuffers(VkDevice inDevice, VkRenderPass inRenderPass);

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);

	static bool findSupportedFormat(VkPhysicalDevice inPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat& outFormat);
	static bool findDepthFormat(VkPhysicalDevice inPhysicalDevice, VkFormat& outFormat);

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& inAvailableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& inAvailablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& inCapabilities);
};