#include "RenderBackend_Vulkan/RenderSwapchain.h"

#include <algorithm>
#include <iostream>

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderQueueFamily.h"

bool RenderSwapchain::initialize(const RenderDevice& inDevice, VkSurfaceKHR inSurface)
{
	device = inDevice.getDevice();

	SwapchainSupportDetails swapchainSupport = querySwapchainSupport(inDevice.getPhysicalDevice(), inSurface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
	imageFormat = surfaceFormat.format;
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
	extent = chooseSwapExtent(swapchainSupport.capabilities);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = inSurface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices = inDevice.getQueueFamilyIndices();
	uint32_t imageHandlingQueueFamilyIndices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = imageHandlingQueueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(inDevice.getDevice(), &swapchainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateSwapchain failed" << std::endl;
		return false;
	}

	// note: unnecessary ? we already have image count. can vkcreateswapchain change the imagecount ?
	//vkGetSwapchainImagesKHR(inDevice.getDevice(), swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(inDevice.getDevice(), swapchain, &imageCount, images.data());

	return createImageViews(inSurface);
}

void RenderSwapchain::shutdown()
{
	for (auto imageView : getImageViews())
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

bool RenderSwapchain::createImageViews(VkSurfaceKHR inSurface)
{
	imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); ++i)
	{
		if (!RenderImage::createImageView(device, images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, imageViews[i]))
		{
			return false;
		}
	}

	return true;
}

RenderSwapchain::SwapchainSupportDetails RenderSwapchain::querySwapchainSupport(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface) 
{
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(inPhysicalDevice, inSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(inPhysicalDevice, inSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(inPhysicalDevice, inSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(inPhysicalDevice, inSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(inPhysicalDevice, inSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR RenderSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& inAvailableFormats)
{
	for (const auto& availableFormat : inAvailableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return inAvailableFormats[0];
}

VkPresentModeKHR RenderSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& inAvailablePresentModes)
{
	for (const auto& availablePresentMode : inAvailablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& inCapabilities)
{
	if (inCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return inCapabilities.currentExtent;
	}
	else
	{
		int width, height;
		platform::getWindowManager().getFramebufferSize(width, height);

		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

		actualExtent.width = std::clamp(actualExtent.width, inCapabilities.minImageExtent.width, inCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, inCapabilities.minImageExtent.height, inCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}