#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <stddef.h>
#include <string>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/vulkan.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

#include "Core/Ensure.h"
#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Core/FileSystem.h"
#include "Core/ResourcesManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "Render/GPUHandle.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/CommandContext.h"
#include "RenderBackend_Vulkan/GeometryRenderPass.h"
#include "RenderBackend_Vulkan/RenderBuffer.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderInstance.h"
#include "RenderBackend_Vulkan/RenderPipeline.h"
#include "RenderBackend_Vulkan/RenderSurface.h"
#include "RenderBackend_Vulkan/RenderSwapchain.h"

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct MyVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

struct RenderBackend::Impl
{
	RenderInstance instance;
	RenderSurface surface;

	RenderDevice device;
	RenderSwapchain swapchain;

	GeometryRenderPass geometryRenderPass;

	RenderPipeline graphicsPipeline;

	CommandContext graphicsCommandContext;
	CommandContext transferCommandContext;
	CommandContext presentCommandContext;

	// scene
	GPUHandle vertexBuffer;
	GPUHandle indexBuffer;
	std::vector<GPUHandle> uniformBuffers;
	std::vector<void*> uniformBuffersMapped;
	
	GPUHandle texture;
	VkSampler textureSampler;

	std::vector<VkDescriptorSet> descriptorSets;

	// sync primitives
	std::vector<VkCommandBuffer> graphicsCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrameId = 0;

	bool bFramebufferResized = false;

	const std::vector<MyVertex> vertices =
	{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
};


namespace
{
	bool createVertexBuffer(RenderDevice& inRenderDevice
		, CommandContext& inCommandContext
		, const std::vector<MyVertex>& inVertices
		, GPUHandle& outBufferHandle)
	{
		size_t dataSize = sizeof(inVertices[0]) * inVertices.size();

		const QueueFamilyIndices& indices = inRenderDevice.getQueueFamilyIndices();
		std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		GPUHandle stagingBufferHandle = inRenderDevice.createBuffer(dataSize, usage, queueFamilyIndices, propertyFlags);
		if (!stagingBufferHandle.isValid())
		{
			return false;
		}

		void* data = (void*)inVertices.data();
		if (!inRenderDevice.fillBuffer(stagingBufferHandle, data, dataSize))
		{
			return false;
		}

		usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		queueFamilyIndices.push_back(indices.graphicsFamily.value());
		GPUHandle vertexBufferHandle = inRenderDevice.createBuffer(dataSize, usage, queueFamilyIndices, propertyFlags);
		if (!vertexBufferHandle.isValid())
		{
			return false;
		}

		const RenderBuffer& stagingBuffer = inRenderDevice.getBuffer(stagingBufferHandle);
		const RenderBuffer& vertexBuffer = inRenderDevice.getBuffer(vertexBufferHandle);
		inCommandContext.executeCopyBufferCommand(stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), dataSize);
		inRenderDevice.destroyBuffer(stagingBufferHandle);

		outBufferHandle = vertexBufferHandle;
		return true;
	}

	bool createIndexBuffer(RenderDevice& inRenderDevice
		, CommandContext& inCommandContext
		, const std::vector<uint16_t>& inIndices
		, GPUHandle& outBufferHandle)
	{
		VkDeviceSize bufferSize = sizeof(inIndices[0]) * inIndices.size();
		
		const QueueFamilyIndices& indices = inRenderDevice.getQueueFamilyIndices();
		std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		GPUHandle stagingBufferHandle = inRenderDevice.createBuffer(bufferSize, usageFlags, queueFamilyIndices, propertyFlags);
		if (!stagingBufferHandle.isValid())
		{
			return false;
		}
		
		inRenderDevice.fillBuffer(stagingBufferHandle, (void*)inIndices.data(), bufferSize);

		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		queueFamilyIndices.push_back(indices.graphicsFamily.value());
		GPUHandle indexBufferHandle = inRenderDevice.createBuffer(bufferSize, usageFlags, queueFamilyIndices, propertyFlags);
		if (!indexBufferHandle.isValid())
		{
			return false;
		}

		const RenderBuffer& stagingBuffer = inRenderDevice.getBuffer(stagingBufferHandle);
		const RenderBuffer& indexBuffer = inRenderDevice.getBuffer(indexBufferHandle);
		inCommandContext.executeCopyBufferCommand(stagingBuffer.getBuffer(), indexBuffer.getBuffer(), bufferSize);
		inRenderDevice.destroyBuffer(stagingBufferHandle);
		
		outBufferHandle = indexBufferHandle;
		return true;
	}

	bool createUniformBuffers(RenderDevice& inRenderDevice
		, std::vector<GPUHandle>& outBufferHandles
		, std::vector<void*>& outMappedMemories)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		const QueueFamilyIndices& indices = inRenderDevice.getQueueFamilyIndices();
		std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value(), indices.graphicsFamily.value() };
		constexpr VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		outBufferHandles.resize(MAX_FRAMES_IN_FLIGHT);
		outMappedMemories.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			outBufferHandles[i] = inRenderDevice.createBuffer(bufferSize, usageFlags, queueFamilyIndices, memoryFlags);

			const RenderBuffer& buffer = inRenderDevice.getBuffer(outBufferHandles[i]);
			vkMapMemory(buffer.getDevice(), buffer.getMemory(), 0, bufferSize, 0, &outMappedMemories[i]);
		}

		return true;
	}

	bool createDescriptorSets(RenderDevice& inDevice, VkDescriptorSetLayout inDescriptorSetLayout, uint32_t inDescriptorSetCount, const std::vector<GPUHandle>& inBuffers, VkImageView inTextureImageView, VkSampler inTextureSampler, std::vector<VkDescriptorSet>& outDescriptorSets)
	{
		std::vector<VkDescriptorSetLayout> layouts(inDescriptorSetCount, inDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = inDevice.getMainDescriptorPool();
		allocInfo.descriptorSetCount = inDescriptorSetCount;
		allocInfo.pSetLayouts = layouts.data();

		outDescriptorSets.resize(inDescriptorSetCount);
		VkResult result = vkAllocateDescriptorSets(inDevice.getDevice(), &allocInfo, outDescriptorSets.data());
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateDescriptorSets failed" << std::endl;
			return false;
		}

		for (size_t i = 0; i < inDescriptorSetCount; i++)
		{
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			//
			const RenderBuffer& renderBuffer = inDevice.getBuffer(inBuffers[i]);
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = renderBuffer.getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = outDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			//
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = inTextureImageView;
			imageInfo.sampler = inTextureSampler;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = outDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(inDevice.getDevice(), (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		return true;
	}

	bool createTextureSampler(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSampler& outTextureSampler)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(inPhysicalDevice, &properties);

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkResult result = vkCreateSampler(inDevice, &samplerInfo, nullptr, &outTextureSampler);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateSampler failed" << std::endl;
			return false;
		}

		return true;
	}

	bool recordCommandBuffer(VkCommandBuffer inCommandBuffer
		, RenderPipeline inPipeline
		, GeometryRenderPass inGeometryRenderPass
		, VkExtent2D inSwapchainExtent
		, uint32_t inImageIndex
		, VkBuffer inVertexBuffer
		, uint32_t inIndexCount
		, VkBuffer inIndexBuffer
		, const std::vector<VkDescriptorSet>& inDescriptorSets
		, uint32_t inCurrentFrame)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		VkResult result = vkBeginCommandBuffer(inCommandBuffer, &beginInfo);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkBeginCommandBuffer failed" << std::endl;
			return false;
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = inGeometryRenderPass.getPass();
		renderPassInfo.framebuffer = inGeometryRenderPass.getFramebuffers()[inImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = inSwapchainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0, 0, 0, 1 };
		clearValues[1].depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(inCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline.getPipeline());

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(inSwapchainExtent.width);
			viewport.height = static_cast<float>(inSwapchainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(inCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = inSwapchainExtent;
			vkCmdSetScissor(inCommandBuffer, 0, 1, &scissor);

			const ComponentManager<MeshRendererComponent>& meshes = engine::getScene()->getMeshRendererComponents();
			for (size_t i = 0; i < meshes.size(); ++i)
			{
				const auto& mesh = meshes.at(i);
				const Model* model = engine::getResources()->getModel(mesh->model);

				VkBuffer vertexBuffers[] = { inVertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(inCommandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(inCommandBuffer, inIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

				vkCmdBindDescriptorSets(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline.getPipelineLayout(), 0, 1, &inDescriptorSets[inCurrentFrame], 0, nullptr);

				vkCmdDrawIndexed(inCommandBuffer, inIndexCount, 1, 0, 0, 0);
			}

		vkCmdEndRenderPass(inCommandBuffer);

		result = vkEndCommandBuffer(inCommandBuffer);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkEndCommandBuffer failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createSyncObjects(VkDevice inDevice, std::vector<VkSemaphore>& outImageAvailableSemaphores, std::vector<VkSemaphore>& outRenderFinishedSemaphores, std::vector<VkFence>& outInFlightFences)
	{
		outImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		outRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		outInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkResult result = vkCreateSemaphore(inDevice, &semaphoreInfo, nullptr, &outImageAvailableSemaphores[i]);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Error: vkCreateSemaphore failed" << std::endl;
				return false;
			}

			result = vkCreateSemaphore(inDevice, &semaphoreInfo, nullptr, &outRenderFinishedSemaphores[i]);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Error: vkCreateSemaphore failed" << std::endl;
				return false;
			}

			result = vkCreateFence(inDevice, &fenceInfo, nullptr, &outInFlightFences[i]);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Error: vkCreateFence failed" << std::endl;
				return false;
			}
		}

		return true;
	}

	unsigned int getVkPrimitiveMode(EPrimitiveMode inMode)
	{
		switch (inMode)
		{
		case EPrimitiveMode::POINTS:
			return 0;
			break;
		case EPrimitiveMode::LINES:
			return 0;
			break;
		case EPrimitiveMode::TRIANGLES:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkBufferType(EBufferType inType)
	{
		switch (inType)
		{
		case EBufferType::VERTEX_ARRAY:
			ensure(false);
			return 0; // should not be needed
			break;
		case EBufferType::VERTICES:
			return 0;
			break;
		case EBufferType::INDICES:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkBufferUsage(EBufferUsage inUsage)
	{
		switch (inUsage)
		{
		case EBufferUsage::STATIC:
			return 0;
			break;
		case EBufferUsage::STREAM:
			return 0;
			break;
		case EBufferUsage::DYNAMIC:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getVkComparisonFunc(EComparisonFunc inFunc)
	{
		switch (inFunc)
		{
		case EComparisonFunc::NEVER:
			return 0;
			break;
		case EComparisonFunc::LESS:
			return 0;
			break;
		case EComparisonFunc::LEQUAL:
			return 0;
			break;
		case EComparisonFunc::EQUAL:
			return 0;
			break;
		case EComparisonFunc::NOTEQUAL:
			return 0;
			break;
		case EComparisonFunc::GEQUAL:
			return 0;
			break;
		case EComparisonFunc::GREATER:
			return 0;
			break;
		case EComparisonFunc::ALWAYS:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	unsigned int getOpenGLTextureFormat(ETextureFormat inTextureFormat)
	{
		switch (inTextureFormat)
		{
		case ETextureFormat::RGB:
			return 0;
			break;
		case ETextureFormat::RGBA:
			return 0;
			break;
		}
		ensure(false);
		return 0;
	}

	void recreateSwapchain(RenderDevice& inDevice, VkSurfaceKHR inSurface, RenderSwapchain& inSwapchain, GeometryRenderPass& inRenderPass, CommandContext& inTransferCommandContext)
	{
		vkDeviceWaitIdle(inDevice.getDevice());

		inRenderPass.free(inDevice);
		inSwapchain.shutdown();

		inSwapchain.initialize(inDevice, inSurface);
		inRenderPass.createDepthResources(inDevice, inSwapchain.getExtent(), inTransferCommandContext);
		inRenderPass.createFramebuffers(inDevice, inSwapchain);
	}

	void updateUniformBuffer(VkExtent2D inExtent, void* inMapped)
	{
		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1), (float)platform::getTime() * glm::radians(90.0f), glm::vec3(0, 0, 1));
		ubo.view = render::getRenderer()->getActiveCamera().getViewMatrix();
		ubo.proj = render::getRenderer()->getActiveCamera().getProjectionMatrix();
		ubo.proj[1][1] *= -1;
		memcpy(inMapped, &ubo, sizeof(ubo));
	}
}

RenderBackend::RenderBackend()
{
	// needs to be located following the renderbackend instance
	impl = new Impl();
}

void RenderBackend::getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints)
{
	outHints.push_back({ contextHints::Requirements::ClientAPI, (uint8_t)contextHints::EClientAPI::NONE });
}

bool RenderBackend::initialize()
{
	if (!impl->instance.initialize())
	{
		return false;
	}

	if (!impl->surface.create(impl->instance.getInstance()))
	{
		return false;
	}

	uint32_t maxDescriptorSets = 2;
	if (!impl->device.initialize(impl->instance.getInstance(), impl->surface.getSurface(), impl->instance.isValidationEnabled(), validationLayers, maxDescriptorSets))
	{
		return false;
	}

	if (!impl->swapchain.initialize(impl->device, impl->surface.getSurface()))
	{
		return false;
	}

	// Command contexts
	const QueueFamilyIndices queueFamilyIndices = impl->device.getQueueFamilyIndices();

	const uint32_t graphicsFamily = queueFamilyIndices.graphicsFamily.value();
	impl->graphicsCommandContext.create(impl->device.getDevice(), graphicsFamily);
	if (!impl->graphicsCommandContext.createCommandBuffers(MAX_FRAMES_IN_FLIGHT, impl->graphicsCommandBuffers))
	{
		return false;
	}

	const uint32_t transferFamily = queueFamilyIndices.transferFamily.value();
	impl->transferCommandContext.create(impl->device.getDevice(), transferFamily);

	const uint32_t presentFamily = queueFamilyIndices.presentFamily.value();
	impl->presentCommandContext.create(impl->device.getDevice(), presentFamily);

	// render pass creation
	if (!impl->geometryRenderPass.initialize(impl->device, impl->swapchain))
	{
		return false;
	}

	if (!impl->geometryRenderPass.createDepthResources(impl->device, impl->swapchain.getExtent(), impl->graphicsCommandContext))
	{
		return false;
	}

	if (!impl->geometryRenderPass.createFramebuffers(impl->device, impl->swapchain))
	{
		return false;
	}

	std::vector<DescriptorSetLayoutBindingDesc> descriptorSetLayoutBindings;
	descriptorSetLayoutBindings.push_back({ 0, DescriptorType::UNIFORM_BUFFER, ShaderStageFlagBits::VERTEX });
	descriptorSetLayoutBindings.push_back({ 1, DescriptorType::TEXTURE_SAMPLER, ShaderStageFlagBits::FRAGMENT });
	
	std::vector<ShaderStageInfo> shaderStageInfos;
	shaderStageInfos.push_back({ "Resources/Shaders/Vulkan/default.vert.spv", ShaderStageFlagBits::VERTEX });
	shaderStageInfos.push_back({ "Resources/Shaders/Vulkan/unlit.frag.spv", ShaderStageFlagBits::FRAGMENT });
	uint32_t vertexSize = sizeof(MyVertex);
	std::vector<VertexInputAttributeDesc> vertexInputAttributeDesc;
	vertexInputAttributeDesc.push_back({ DataFormat::R32G32B32, offsetof(MyVertex, pos) }); // pos
	vertexInputAttributeDesc.push_back({ DataFormat::R32G32B32, offsetof(MyVertex, color) }); // color
	vertexInputAttributeDesc.push_back({ DataFormat::R32G32, offsetof(MyVertex, texCoord) }); // tex

	if (!impl->graphicsPipeline.initialize(impl->device.getDevice()
		, impl->geometryRenderPass.getPass()
		, impl->swapchain.getExtent()
		, descriptorSetLayoutBindings
		, shaderStageInfos
		, vertexSize
		, vertexInputAttributeDesc))
	{
		return false;
	}

	// scene
	ResourceHandle textureHandle = engine::getResources()->loadTexture(File::getResourcesDir() + "Textures/texture.jpg", "diffuse");
	const Texture* texture = engine::getResources()->getTexture(textureHandle);
	impl->texture = texture->getHandle();
	if (!impl->texture.isValid())
	{
		return false;
	}

	if (!createTextureSampler(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->textureSampler))
	{
		return false;
	}

	if (!createVertexBuffer(impl->device, impl->transferCommandContext, impl->vertices, impl->vertexBuffer))
	{
		return false;
	}

	if (!createIndexBuffer(impl->device, impl->transferCommandContext, impl->indices, impl->indexBuffer))
	{
		return false;
	}

	if (!createUniformBuffers(impl->device, impl->uniformBuffers, impl->uniformBuffersMapped))
	{
		return false;
	}

	const RenderImage& image = impl->device.getImage(impl->texture);
	if (!createDescriptorSets(impl->device, impl->graphicsPipeline.getDescriptorSetLayout(), maxDescriptorSets, impl->uniformBuffers, image.getView(), impl->textureSampler, impl->descriptorSets))
	{
		return false;
	}

	// Commands
	if (!createSyncObjects(impl->device.getDevice(), impl->imageAvailableSemaphores, impl->renderFinishedSemaphores, impl->inFlightFences))
	{
		return false;
	}

	return true;
}

void RenderBackend::shutdown()
{
	vkDeviceWaitIdle(impl->device.getDevice());

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(impl->device.getDevice(), impl->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(impl->device.getDevice(), impl->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(impl->device.getDevice(), impl->inFlightFences[i], nullptr);
	}

	impl->swapchain.shutdown();

	vkDestroySampler(impl->device.getDevice(), impl->textureSampler, nullptr);
	impl->device.destroyImage(impl->texture);
	impl->device.destroyBuffer(impl->vertexBuffer);
	impl->device.destroyBuffer(impl->indexBuffer);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		impl->device.destroyBuffer(impl->uniformBuffers[i]);
	}

	impl->graphicsPipeline.destroy();

	impl->geometryRenderPass.free(impl->device);

	impl->graphicsCommandContext.free();
	impl->transferCommandContext.free();
	impl->presentCommandContext.free();

	impl->device.shutdown();

	impl->surface.destroy(impl->instance.getInstance());
	impl->instance.shutdown();
}

std::string RenderBackend::getName()
{
	return "Vulkan";
}

void RenderBackend::tracyGPUCollect()
{}

void RenderBackend::drawGeometry(Scene& inScene)
{
	vkWaitForFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(impl->device.getDevice(), impl->swapchain.get(), UINT64_MAX, impl->imageAvailableSemaphores[impl->currentFrameId], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain(impl->device, impl->surface.getSurface(), impl->swapchain, impl->geometryRenderPass, impl->transferCommandContext);
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cerr << "Error: vkAcquiredNextImageKHR failed" << std::endl;
		return;
	}

	vkResetFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId]);

	updateUniformBuffer(impl->swapchain.getExtent(), impl->uniformBuffersMapped[impl->currentFrameId]);

	vkResetCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId], 0);

	const RenderBuffer& vertexBuffer = impl->device.getBuffer(impl->vertexBuffer);
	const RenderBuffer& indexBuffer = impl->device.getBuffer(impl->indexBuffer);
	recordCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId]
		, impl->graphicsPipeline
		, impl->geometryRenderPass
		, impl->swapchain.getExtent()
		, imageIndex
		, vertexBuffer.getBuffer()
		, (uint32_t)impl->indices.size()
		, indexBuffer.getBuffer()
		, impl->descriptorSets
		, impl->currentFrameId);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { impl->imageAvailableSemaphores[impl->currentFrameId] };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &impl->graphicsCommandBuffers[impl->currentFrameId];

	VkSemaphore signalSemaphores[] = { impl->renderFinishedSemaphores[impl->currentFrameId] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = vkQueueSubmit(impl->graphicsCommandContext.getQueue(), 1, &submitInfo, impl->inFlightFences[impl->currentFrameId]);
	if (result != VK_SUCCESS)
	{
		return;
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { impl->swapchain.get() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(impl->presentCommandContext.getQueue(), &presentInfo);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || impl->bFramebufferResized)
	{
		impl->bFramebufferResized = false;
		recreateSwapchain(impl->device, impl->surface.getSurface(), impl->swapchain, impl->geometryRenderPass, impl->transferCommandContext);
	}
	else if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkQueuePresentKHR failed" << std::endl;
		return;
	}

	frameNumber++;
	impl->currentFrameId = frameNumber % MAX_FRAMES_IN_FLIGHT;
}

void RenderBackend::clear()
{
	ZoneScopedN("Renderer::clear");
}

void RenderBackend::resizeViewport(int inX, int inY, int inWidth, int inHeight)
{
	impl->bFramebufferResized = true;
}

void RenderBackend::setStencilMask(unsigned int inMask)
{
}

void RenderBackend::setStencilFunc(EComparisonFunc inFunc, int inValue, unsigned int inMask)
{
}

GPUHandle RenderBackend::compileShader(std::string_view inVertexPath, std::string_view inFragmentPath)
{
	return {};
}

void RenderBackend::useShaderProgram(GPUHandle inHandle)
{
	ZoneScoped;
}

void RenderBackend::setBool(std::string_view inName, bool inValue)
{
}

void RenderBackend::setInt(std::string_view inName, int inValue)
{
}

void RenderBackend::setFloat(std::string_view inName, float inValue)
{
}

void RenderBackend::setVec3(std::string_view inName, const glm::vec3& inValue)
{
}

void RenderBackend::setVec4(std::string_view inName, float x, float y, float z, float w)
{
}

void RenderBackend::setMat4(std::string_view inName, glm::f32* inValue)
{
}

void RenderBackend::setTexture(std::string_view inName, GPUHandle inValue)
{
}

//
void RenderBackend::drawArrays(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount)
{
}

void RenderBackend::drawArraysInstanced(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount)
{
}

void RenderBackend::drawElements(GPUHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices)
{
}

void RenderBackend::drawMesh(const Mesh& inMesh)
{
}


//
GPUHandle RenderBackend::createTexture(uint8_t* inData, size_t inWidth, size_t inHeight)
{
	// note: texture loading forces RGBA so 4 channels
	VkDeviceSize imageSize = inWidth * inHeight * 4;

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const QueueFamilyIndices& indices = impl->device.getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
	GPUHandle dataBufferHandle = impl->device.createBuffer(imageSize, usage, queueFamilyIndices, properties);
	if (!dataBufferHandle.isValid())
	{
		return false;
	}

	impl->device.fillBuffer(dataBufferHandle, (void*)inData, imageSize);

	// note: format forced at texture level
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUHandle imageHandle = impl->device.createImage((uint32_t)inWidth, (uint32_t)inHeight, format, tiling, usage, queueFamilyIndices, properties);
	if (!imageHandle.isValid())
	{
		return false;
	}

	VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const RenderBuffer& renderBuffer = impl->device.getBuffer(dataBufferHandle);
	RenderImage& renderImage = impl->device.getImage(imageHandle);
	impl->transferCommandContext.executeTransitionImageLayoutCommand(renderImage.getImage(), format, oldLayout, newLayout);
	impl->transferCommandContext.executeCopyBufferToImageCommand(renderBuffer.getBuffer(), renderImage.getImage(), (uint32_t)inWidth, (uint32_t)inHeight);

	oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	impl->graphicsCommandContext.executeTransitionImageLayoutCommand(renderImage.getImage(), format, oldLayout, newLayout);
	impl->device.destroyBuffer(dataBufferHandle);

	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageView view = RenderImage::createImageView(impl->device.getDevice(), renderImage.getImage(), format, aspectFlags);
	renderImage.setTextureView(view);

	return imageHandle;
}

void RenderBackend::bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex)
{
}

void RenderBackend::deleteTexture(GPUHandle inHandle)
{
	impl->device.destroyImage(inHandle);
}

//
GPUHandle RenderBackend::createGPUBuffer(EBufferType inType)
{
	return {};
}

void RenderBackend::allocateGPUBuffer(EBufferType inType, size_t inCapacity, void* inData, EBufferUsage inUsage)
{
}

void RenderBackend::bindGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
}

void RenderBackend::deleteGPUBuffer(EBufferType inType, GPUHandle inHandle)
{
}

void RenderBackend::setGPUBufferData(EBufferType inType, size_t inOffset, size_t inSize, void* inData)
{
}

void RenderBackend::setVertexLayoutData(unsigned int inLayoutIndex, size_t inSizeInBytes, size_t inVertexStride, void* inDataOffset, unsigned int inDivisor)
{
}


namespace render
{
	RenderBackend* g_renderBackEnd = nullptr;

	bool initializeBackend()
	{
		g_renderBackEnd = new RenderBackend();
		return g_renderBackEnd->initialize();
	}

	void shutdownBackend()
	{
		if (g_renderBackEnd)
		{
			g_renderBackEnd->shutdown();
			delete g_renderBackEnd;
		}
		g_renderBackEnd = nullptr;
	}

	RenderBackend* getRenderBackend()
	{
		return g_renderBackEnd;
	}
}