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
#include "Core/Resources/Material.h"
#include "Core/Resources/MaterialInstance.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/EntityManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "Render/GPUHandle.h"
#include "Render/Renderer.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/VulkanCommandQueue.h"
#include "RenderBackend_Vulkan/VulkanGeometryRenderPass.h"
#include "RenderBackend_Vulkan/VulkanBuffer.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"
#include "RenderBackend_Vulkan/VulkanImage.h"
#include "RenderBackend_Vulkan/VulkanInstance.h"
#include "RenderBackend_Vulkan/VulkanPipeline.h"
#include "RenderBackend_Vulkan/VulkanSurface.h"
#include "RenderBackend_Vulkan/VulkanSwapchain.h"

struct RenderBackend::Impl
{
	VulkanInstance instance;
	VulkanSurface surface;

	VulkanDevice device;
	VulkanSwapchain swapchain;

	VulkanGeometryRenderPass geometryRenderPass;
	VulkanPipeline geometryPipeline;

	// sync primitives
	std::vector<VkCommandBuffer> graphicsCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrameId = 0;
	bool bFramebufferResized = false;
};

namespace
{
	bool beginCommandBuffer(VkCommandBuffer inCommandBuffer)
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

		return true;
	}

	bool endCommandBuffer(VkCommandBuffer inCommandBuffer)
	{
		VkResult result = vkEndCommandBuffer(inCommandBuffer);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkEndCommandBuffer failed" << std::endl;
			return false;
		}
		return true;
	}

	void bindRenderPass(VkCommandBuffer inCommandBuffer
		, VulkanGeometryRenderPass inGeometryRenderPass
		, VkExtent2D inSwapchainExtent
		, uint32_t inImageIndex)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = inGeometryRenderPass.getRenderPass();
		renderPassInfo.framebuffer = inGeometryRenderPass.getFramebuffers()[inImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = inSwapchainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0, 0, 0, 1 };
		clearValues[1].depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(inCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void unbindRenderPass(VkCommandBuffer inCommandBuffer)
	{
		vkCmdEndRenderPass(inCommandBuffer);
	}

	void bindPipeline(VkCommandBuffer inCommandBuffer
		, VulkanPipeline inGeometryPipeline
		, VkExtent2D inSwapchainExtent)
	{
		vkCmdBindPipeline(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inGeometryPipeline.getPipeline());

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
	}

	BufferHandle pushFrameData(VulkanDevice& inDevice, uint32_t inFrameId)
	{
		FrameData frameData{};
		frameData.view = render::getRenderer()->getActiveCamera().getViewMatrix();
		frameData.proj = render::getRenderer()->getActiveCamera().getProjectionMatrix();
		frameData.proj[1][1] *= -1;

		return inDevice.pushFrameData((void*)&frameData, sizeof(FrameData), inFrameId);
	}

	void bindDescriptorSets(VulkanDevice& inDevice, VkCommandBuffer inCommandBuffer, VkPipelineLayout inPipelineLayout, BufferHandle inFrameData, VkDescriptorSet inFrameBuffer, const MaterialInstance& inMaterialInstance)
	{
		std::vector<uint32_t> dynamicOffsets;
		std::vector<VkDescriptorSet> descriptorSets;

		// Set 0 : Frame Data
		dynamicOffsets.push_back(inFrameData.offset);
		descriptorSets.push_back(inFrameBuffer);

		// Set 1 : Texture Data
		descriptorSets.push_back(inDevice.getTextureDescriptorSet());

		// Set 2 : Material Data
		dynamicOffsets.push_back(inMaterialInstance.getMaterialBufferHandle().offset);
		descriptorSets.push_back(inDevice.getMaterialBufferDescriptorSet());

		vkCmdBindDescriptorSets(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), (uint32_t)dynamicOffsets.size(), dynamicOffsets.data());
	}

	// todo
	const VulkanPipeline* getPipelineForMaterial(const Material* inMaterial, const VulkanPipeline* inVulkanPipeline)
	{
		return inVulkanPipeline;
	}

	BufferHandle pushInstanceData(VulkanDevice& inDevice, uint32_t inFrameId, const glm::mat4& inModelMat)
	{
		InstanceData instanceData{};
		//instanceData.model = glm::rotate(inModelMat, (float)platform::getTime() * glm::radians(90.0f), glm::vec3(0, 0, 1));
		instanceData.model = inModelMat;

		return inDevice.pushFrameData((void*)&instanceData, sizeof(InstanceData), inFrameId);
	}

	bool recordCommandBuffer(VulkanDevice& inDevice
		, VkCommandBuffer inCommandBuffer
		, const VulkanPipeline& inGeometryPipeline
		, const VulkanGeometryRenderPass& inGeometryRenderPass
		, VkExtent2D inSwapchainExtent
		, uint32_t inImageIndex
		, uint32_t inFrameId)
	{
		beginCommandBuffer(inCommandBuffer);
		bindRenderPass(inCommandBuffer, inGeometryRenderPass, inSwapchainExtent, inImageIndex);

		const BufferHandle frameDataHandle = pushFrameData(inDevice, inFrameId);
		VkDescriptorSet frameBufferDescriptorSet = inDevice.getFrameBufferDescriptorSet(inFrameId);
		VulkanBuffer& frameBuffer = inDevice.getBuffer(inDevice.getFrameBufferHandle(inFrameId));

		const VulkanPipeline* boundPipeline = nullptr;
		const ComponentManager<MeshRendererComponent>& meshRenderers = engine::getScene()->getMeshRendererComponents();
		const ComponentManager<WorldTransformComponent>& worldTransforms = engine::getScene()->getWorldTransformComponents();
		for (size_t i = 0; i < meshRenderers.size(); ++i)
		{
			const MeshRendererComponent* meshRenderer = meshRenderers.at(i);

			ResourceHandle materialInstanceHandle = meshRenderer->materialInstance;
			MaterialInstance* materialInstance = engine::getResources()->getMaterialInstance(materialInstanceHandle);
			ensure(materialInstance);

			Material* material = engine::getResources()->getMaterial(materialInstance->getMaterial());
			ensure(material);

			const VulkanPipeline* pipelineNeeded = getPipelineForMaterial(material, &inGeometryPipeline);
			if (boundPipeline != pipelineNeeded)
			{
				ensure(pipelineNeeded != nullptr);
				bindPipeline(inCommandBuffer, *pipelineNeeded, inSwapchainExtent);
				boundPipeline = pipelineNeeded;
				
				bindDescriptorSets(inDevice, inCommandBuffer, boundPipeline->getPipelineLayout(), frameDataHandle, frameBufferDescriptorSet, *materialInstance);
			}

			const Model* model = engine::getResources()->getModel(meshRenderer->model);
			ensure(model);
			const std::vector<Mesh>& meshes = model->getMeshes();

			const WorldTransformComponent* worldTransform = worldTransforms.get(meshRenderer->entity);
			ensure(worldTransform);
			for (size_t j = 0; j < meshes.size(); ++j)
			{
				//const VulkanBuffer& vertexBuffer = inDevice.getBuffer(meshes[j].getVertexBuffer());
				//const VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer() };
				//const VkDeviceSize offsets[] = { 0 };
				//vkCmdBindVertexBuffers(inCommandBuffer, 0, 1, vertexBuffers, offsets);

				//const VulkanBuffer& indexBuffer = inDevice.getBuffer(meshes[j].getIndexBuffer());
				//vkCmdBindIndexBuffer(inCommandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

				//VkShaderStageFlags stageFlags = getShaderStageFlags(EShaderStageFlagBits::VERTEX);

				//BufferHandle instanceDataHandle = pushInstanceData(inDevice, inFrameId, worldTransform->model);
				//uint8_t* dataPtr = (uint8_t*)frameBuffer.getMappedMemory() + instanceDataHandle.offset;
				//vkCmdPushConstants(inCommandBuffer, inGeometryPipeline.getPipelineLayout(), stageFlags, 0, instanceDataHandle.size, dataPtr);

				//vkCmdDrawIndexed(inCommandBuffer, (uint32_t)meshes[j].getIndicesSize(), 1, 0, 0, 0);
			}
		}

		unbindRenderPass(inCommandBuffer);
		return endCommandBuffer(inCommandBuffer);
	}

	bool createSyncObjects(VkDevice inDevice, std::vector<VkSemaphore>& outImageAvailableSemaphores, std::vector<VkSemaphore>& outRenderFinishedSemaphores, std::vector<VkFence>& outInFlightFences)
	{
		outImageAvailableSemaphores.resize(renderConfig::getFrameCount());
		outRenderFinishedSemaphores.resize(renderConfig::getFrameCount());
		outInFlightFences.resize(renderConfig::getFrameCount());

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < renderConfig::getFrameCount(); ++i)
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

	void recreateSwapchain(VulkanDevice& inDevice, VkSurfaceKHR inSurface, VulkanSwapchain& inSwapchain, VulkanGeometryRenderPass& inRenderPass)
	{
		vkDeviceWaitIdle(inDevice.getDevice());

		inRenderPass.free(inDevice);
		inSwapchain.shutdown();

		inSwapchain.initialize(inDevice, inSurface);
		inRenderPass.initialize(inDevice, inSwapchain);
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

VulkanDevice& RenderBackend::getDevice()
{
	return impl->device;
}

VulkanPipeline& RenderBackend::getGeometryPipeline()
{
	return impl->geometryPipeline;
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

	if (!impl->device.initialize(impl->instance, impl->surface.getSurface()))
	{
		return false;
	}

	if (!impl->swapchain.initialize(impl->device, impl->surface.getSurface()))
	{
		return false;
	}

	if (!impl->device.getGraphicsVulkanCommandQueue().createCommandBuffers(renderConfig::getFrameCount(), impl->graphicsCommandBuffers))
	{
		return false;
	}

	// render pass creation
	if (!impl->geometryRenderPass.initialize(impl->device, impl->swapchain))
	{
		return false;
	}

	// note: all of this pipeline creation thing should be in consequence of a material being loaded, so material infos would already be obtainable

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(InstanceData);
	pushConstantRange.stageFlags = getShaderStageFlags(EShaderStageFlagBits::VERTEX);
	const std::vector<VkPushConstantRange> pushConstantRanges = { pushConstantRange };

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.push_back(impl->device.getFrameBufferDescriptorSetLayout());
	descriptorSetLayouts.push_back(impl->device.getTextureDescriptorSetLayout());
	descriptorSetLayouts.push_back(impl->device.getMaterialDescriptorSetLayout());

	std::vector<ShaderStageInfo> shaderStageInfos = shader::getUnlitMaterialStageInfos();
	uint32_t vertexSize = shader::getUnlitMaterialVertexSize();
	std::vector<VertexInputAttributeDesc> vertexInputAttributeDesc = shader::getUnlitMaterialVertexInputs();

	if (!impl->geometryPipeline.initialize(impl->device
		, impl->geometryRenderPass.getRenderPass()
		, impl->swapchain.getExtent()
		, shaderStageInfos
		, vertexInputAttributeDesc
		, vertexSize
		, descriptorSetLayouts
		, pushConstantRanges))
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

	for (size_t i = 0; i < renderConfig::getFrameCount(); ++i)
	{
		vkDestroySemaphore(impl->device.getDevice(), impl->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(impl->device.getDevice(), impl->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(impl->device.getDevice(), impl->inFlightFences[i], nullptr);
	}

	impl->swapchain.shutdown();

	impl->geometryPipeline.destroy();
	impl->geometryRenderPass.free(impl->device);

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
		recreateSwapchain(impl->device, impl->surface.getSurface(), impl->swapchain, impl->geometryRenderPass);
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cerr << "Error: vkAcquiredNextImageKHR failed" << std::endl;
		return;
	}

	vkResetFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId]);

	impl->device.resetFrameData(impl->currentFrameId);

	vkResetCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId], 0);

	recordCommandBuffer(impl->device
		, impl->graphicsCommandBuffers[impl->currentFrameId]
		, impl->geometryPipeline
		, impl->geometryRenderPass
		, impl->swapchain.getExtent()
		, imageIndex
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

	result = vkQueueSubmit(impl->device.getGraphicsVulkanCommandQueue().getQueue(), 1, &submitInfo, impl->inFlightFences[impl->currentFrameId]);
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

	vkQueuePresentKHR(impl->device.getPresentVulkanCommandQueue().getQueue(), &presentInfo);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || impl->bFramebufferResized)
	{
		impl->bFramebufferResized = false;
		recreateSwapchain(impl->device, impl->surface.getSurface(), impl->swapchain, impl->geometryRenderPass);
	}
	else if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkQueuePresentKHR failed" << std::endl;
		return;
	}

	frameNumber++;
	impl->currentFrameId = frameNumber % renderConfig::getFrameCount();
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

GPUAllocHandle RenderBackend::compileShader(std::string_view inVertexPath, std::string_view inFragmentPath)
{
	return GPUAllocHandle::invalid;
}

void RenderBackend::useShaderProgram(GPUAllocHandle inHandle)
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

void RenderBackend::setTexture(std::string_view inName, GPUAllocHandle inValue)
{
}

//
void RenderBackend::drawArrays(GPUAllocHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inCount)
{
}

void RenderBackend::drawArraysInstanced(GPUAllocHandle inVAO, EPrimitiveMode inPrimitiveMode, int inOffset, size_t inSize, size_t inCount)
{
}

void RenderBackend::drawElements(GPUAllocHandle inVAO, EPrimitiveMode inPrimitiveMode, size_t inCount, void* inIndices)
{
}

void RenderBackend::drawMesh(const Mesh& inMesh)
{
}

//
GPUAllocHandle RenderBackend::createTexture(const uint8_t* inData, size_t inWidth, size_t inHeight)
{
	// note: texture loading forces RGBA so 4 channels
	VkDeviceSize imageSize = inWidth * inHeight * 4;

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const QueueFamilyIndices& indices = impl->device.getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
	GPUAllocHandle dataBufferHandle = impl->device.createBuffer(imageSize, usage, queueFamilyIndices, properties);
	if (!dataBufferHandle.isValid())
	{
		return false;
	}

	VulkanBuffer& VulkanBuffer = impl->device.getBuffer(dataBufferHandle);
	VulkanBuffer.mapAndFill((void*)inData, imageSize);

	// note: format forced at texture level
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUAllocHandle imageHandle = impl->device.createImage((uint32_t)inWidth, (uint32_t)inHeight, format, tiling, usage, queueFamilyIndices, properties);
	if (!imageHandle.isValid())
	{
		return false;
	}

	VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	VulkanImage& VulkanImage = impl->device.getImage(imageHandle);
	impl->device.getTransferVulkanCommandQueue().executeTransitionImageLayoutCommand(VulkanImage.getImage(), format, oldLayout, newLayout);
	impl->device.getTransferVulkanCommandQueue().executeCopyBufferToImageCommand(VulkanBuffer.getBuffer(), VulkanImage.getImage(), (uint32_t)inWidth, (uint32_t)inHeight);

	oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	impl->device.getGraphicsVulkanCommandQueue().executeTransitionImageLayoutCommand(VulkanImage.getImage(), format, oldLayout, newLayout);
	impl->device.destroyBuffer(dataBufferHandle);

	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageView view = VulkanImage::createImageView(impl->device.getDevice(), VulkanImage.getImage(), format, aspectFlags);
	VulkanImage.setTextureView(view);

	return imageHandle;
}

void RenderBackend::bindTexture(GPUAllocHandle inHandle, std::string_view inType, size_t inIndex)
{
}

void RenderBackend::deleteTexture(GPUAllocHandle inHandle)
{
	impl->device.destroyImage(inHandle);
}

//
GPUAllocHandle RenderBackend::createGPUBuffer(EBufferType inType)
{
	return GPUAllocHandle::invalid;
}

void RenderBackend::allocateGPUBuffer(EBufferType inType, size_t inCapacity, void* inData, EBufferUsage inUsage)
{
}

void RenderBackend::bindGPUBuffer(EBufferType inType, GPUAllocHandle inHandle)
{
}

void RenderBackend::deleteGPUBuffer(EBufferType inType, GPUAllocHandle inHandle)
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
	RHICommandRecorder* g_commandRecorder = nullptr;

	bool initializeBackend()
	{
		g_commandRecorder = new RHICommandRecorder();
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

	RHICommandRecorder* getCommandRecorder()
	{
		return g_commandRecorder;
	}
}