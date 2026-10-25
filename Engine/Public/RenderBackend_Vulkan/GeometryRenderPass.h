#pragma once

#include <array>
#include <vulkan/vulkan.h>

#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/CommandContext.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderSwapchain.h"

class GeometryRenderPass
{
	VkDevice device;

	VkRenderPass pass;
	std::vector<VkFramebuffer> framebuffers;
	GPUHandle depthBufferHandle;

public:
	VkRenderPass getPass() const { return pass; }
	const std::vector<VkFramebuffer>& getFramebuffers() const { return framebuffers; }

	bool initialize(const RenderDevice& inDevice, const RenderSwapchain& inSwapchain)
	{ 
		device = inDevice.getDevice();

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = inSwapchain.getImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		GeometryRenderPass::findDepthFormat(inDevice.getPhysicalDevice(), depthAttachment.format);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &pass);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateRenderPass failed" << std::endl;
			return false;
		}
		return true;
	}

	void free(RenderDevice& inDevice)
	{
		inDevice.destroyImage(depthBufferHandle);

		for (auto framebuffer : getFramebuffers())
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyRenderPass(device, pass, nullptr);
	}

	bool createFramebuffers(RenderDevice& inDevice, const RenderSwapchain& inSwapchain)
	{
		const std::vector<VkImageView>& swapchainImageViews = inSwapchain.getImageViews();
		framebuffers.resize(swapchainImageViews.size());

		RenderImage& depthBufferImage = inDevice.getImage(depthBufferHandle);

		for (size_t i = 0; i < swapchainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments =
			{
				swapchainImageViews[i],
				depthBufferImage.getView()
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = pass;
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = inSwapchain.getExtent().width;
			framebufferInfo.height = inSwapchain.getExtent().height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Error: vkCreateFramebuffer failed" << std::endl;
				return false;
			}
		}

		return true;
	}

	bool createDepthResources(RenderDevice& inDevice, const VkExtent2D& inExtent, CommandContext& inCommandContext)
	{
		VkFormat depthFormat;
		GeometryRenderPass::findDepthFormat(inDevice.getPhysicalDevice(), depthFormat);

		QueueFamilyIndices queueFamilyIndices = inDevice.getQueueFamilyIndices();
		depthBufferHandle = inDevice.createImage(inExtent.width, inExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.transferFamily.value() }, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!depthBufferHandle.isValid())
		{
			return false;
		}

		RenderImage& depthBufferImage = inDevice.getImage(depthBufferHandle);
		VkImageView view = RenderImage::createImageView(inDevice.getDevice(), depthBufferImage.getImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		if (view == VK_NULL_HANDLE)
		{
			return false;
		}
		depthBufferImage.setTextureView(view);

		inCommandContext.executeTransitionImageLayoutCommand(depthBufferImage.getImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		return true;
	}

private:
	bool findSupportedFormat(VkPhysicalDevice inPhysicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat& outFormat)
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

	bool findDepthFormat(VkPhysicalDevice inPhysicalDevice, VkFormat& outFormat)
	{
		return GeometryRenderPass::findSupportedFormat(inPhysicalDevice
			, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }
			, VK_IMAGE_TILING_OPTIMAL
			, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			, outFormat);
	}
};