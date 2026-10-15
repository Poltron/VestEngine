#include "RenderBackend_Vulkan/RenderSwapchain.h"

#include <algorithm>
#include <array>
#include <iostream>

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderQueueFamily.h"

bool RenderSwapchain::initialize(RenderDevice& inDevice, VkSurfaceKHR inSurface)
{
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

	RenderDevice::QueueFamilyIndices indices = RenderDevice::findQueueFamilies(inDevice.getPhysicalDevice(), inSurface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
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

	vkGetSwapchainImagesKHR(inDevice.getDevice(), swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(inDevice.getDevice(), swapchain, &imageCount, images.data());

	return true;
}

void RenderSwapchain::shutdown(RenderDevice& inDevice)
{
	vkDestroyImageView(inDevice.getDevice(), depthImageView, nullptr);
	vkDestroyImage(inDevice.getDevice(), depthImage, nullptr);
	vkFreeMemory(inDevice.getDevice(), depthImageMemory, nullptr);

	for (auto framebuffer : getFramebuffers())
	{
		vkDestroyFramebuffer(inDevice.getDevice(), framebuffer, nullptr);
	}

	for (auto imageView : getImageViews())
	{
		vkDestroyImageView(inDevice.getDevice(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(inDevice.getDevice(), swapchain, nullptr);
}

bool RenderSwapchain::createImageViews(VkPhysicalDevice inPhysicalDevice, VkDevice inDevice, VkSurfaceKHR inSurface)
{
	imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); ++i)
	{
		if (!RenderImage::createImageView(inDevice, images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, imageViews[i]))
		{
			return false;
		}
	}

	return true;
}

bool RenderSwapchain::createDepthResources(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkCommandPool inCommandPool, VkQueue inQueue)
{
	VkFormat depthFormat;
	findDepthFormat(inPhysicalDevice, depthFormat);

	if (!RenderImage::createImage(inDevice, inPhysicalDevice, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, {}, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory))
	{
		return false;
	}
	if (!RenderImage::createImageView(inDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView))
	{
		return false;
	}

	RenderQueueFamily::transitionImageLayoutCommand(inDevice, inCommandPool, inQueue, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	return true;
}

bool RenderSwapchain::createFramebuffers(VkDevice inDevice, VkRenderPass inRenderPass)
{
	framebuffers.resize(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = 
		{
			imageViews[i],
			depthImageView
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = inRenderPass;
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(inDevice, &framebufferInfo, nullptr, &framebuffers[i]);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateFramebuffer failed" << std::endl;
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

bool RenderSwapchain::findSupportedFormat(VkPhysicalDevice inPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat& outFormat)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(inPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			outFormat = format;
			return true;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			outFormat = format;
			return true;
		}
	}

	std::cerr << "Error: failed to find supported VkFormat" << std::endl;
	return false;
}

bool RenderSwapchain::findDepthFormat(VkPhysicalDevice inPhysicalDevice, VkFormat& outFormat)
{
	return findSupportedFormat(inPhysicalDevice
		, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }
		, VK_IMAGE_TILING_OPTIMAL
		, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		, outFormat);
}