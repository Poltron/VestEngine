#pragma once

#include <iostream>
#include <vulkan/vulkan.h>

class CommandContext
{
	VkDevice device;

	VkQueue queue;
	uint32_t queueFamilyIndex;
	VkCommandPool commandPool;

public:
	VkQueue getQueue() { return queue; }
	VkCommandPool getCommandPool() { return commandPool; }
	bool isValid() { return device != VK_NULL_HANDLE; }

	void create(VkDevice inDevice, uint32_t inQueueFamilyIndex)
	{
		device = inDevice;

		queueFamilyIndex = inQueueFamilyIndex;
		vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

		createCommandPool(queueFamilyIndex, commandPool);
	}

	void free()
	{
		vkDestroyCommandPool(device, commandPool, nullptr);
	}

	bool createCommandPool(uint32_t inQueueFamily, VkCommandPool& outCommandPool)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = inQueueFamily;

		VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &outCommandPool);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateCommandPool failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createCommandBuffer(VkCommandBuffer& outCommandBuffer)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &outCommandBuffer);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateCommandBuffers failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createCommandBuffers(size_t inCommandBufferCount, std::vector<VkCommandBuffer>& outCommandBuffers)
	{
		outCommandBuffers.resize(inCommandBufferCount);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)outCommandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(device, &allocInfo, outCommandBuffers.data());
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateCommandBuffers failed" << std::endl;
			return false;
		}
		return true;
	}

	void freeCommandBuffer(VkCommandBuffer inCommandBuffer)
	{
		vkFreeCommandBuffers(device, getCommandPool(), 1, &inCommandBuffer);
	}

	VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBuffer commandBuffer;
		createCommandBuffer(commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer inCommandBuffer)
	{
		vkEndCommandBuffer(inCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &inCommandBuffer;

		vkQueueSubmit(getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(getQueue());

		freeCommandBuffer(inCommandBuffer);
	}

	static bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void executeTransitionImageLayoutCommand(VkImage inImage, VkFormat inFormat, VkImageLayout inOldLayout, VkImageLayout inNewLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = inOldLayout;
		barrier.newLayout = inNewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = inImage;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (inNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(inFormat))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (inOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && inNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (inOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && inNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (inOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && inNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			std::cerr << "Error: transitionImageLayout failed" << std::endl;
			endSingleTimeCommands(commandBuffer);
			return;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void executeCopyBufferCommand(VkBuffer inSrcBuffer, VkBuffer inDstBuffer, VkDeviceSize inSize)
	{
		VkCommandBuffer commandBuffer = CommandContext::beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = inSize;
		vkCmdCopyBuffer(commandBuffer, inSrcBuffer, inDstBuffer, 1, &copyRegion);

		CommandContext::endSingleTimeCommands(commandBuffer);
	}

	void executeCopyBufferToImageCommand(VkBuffer inBuffer, VkImage inImage, uint32_t inWidth, uint32_t inHeight)
	{
		VkCommandBuffer commandBuffer = CommandContext::beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { inWidth, inHeight, 1 };

		vkCmdCopyBufferToImage(commandBuffer, inBuffer, inImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		CommandContext::endSingleTimeCommands(commandBuffer);
	}
};