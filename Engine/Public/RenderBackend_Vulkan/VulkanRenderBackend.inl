#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <set>
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
#include "RenderBackend/RHICommandRecorder.h"
#include "RenderBackend_Vulkan/VulkanCommandQueue.h"
#include "RenderBackend_Vulkan/VulkanBuffer.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"
#include "RenderBackend_Vulkan/VulkanGeometryRenderPass.h"
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

	// sync primitives
	std::vector<VkCommandBuffer> graphicsCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrameId = 0;
	bool bFramebufferResized = false;
};

struct PipelineState
{
	uint32_t pipelineHandle = GPURESOURCE_INVALID_ID;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	BufferHandle frameBuffer = BufferHandle::invalid;
	VkDescriptorSet frameDescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
	BufferHandle materialBuffer = BufferHandle::invalid;
	VkDescriptorSet materialDescriptorSet = VK_NULL_HANDLE;

	void reset()
	{
		pipelineHandle = GPURESOURCE_INVALID_ID;
		pipelineLayout = VK_NULL_HANDLE;

		frameBuffer = BufferHandle::invalid;
		frameDescriptorSet = VK_NULL_HANDLE;
		textureDescriptorSet = VK_NULL_HANDLE;
		materialBuffer = BufferHandle::invalid;
		materialDescriptorSet = VK_NULL_HANDLE;
	}
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

	BufferHandle pushFrameData(VulkanDevice& inDevice, VulkanBuffer& inBuffer)
	{
		FrameData frameData{};
		frameData.view = render::getRenderer()->getActiveCamera().getViewMatrix();
		frameData.proj = render::getRenderer()->getActiveCamera().getProjectionMatrix();
		frameData.proj[1][1] *= -1;

		return inDevice.pushFrameData((void*)&frameData, sizeof(FrameData), inBuffer);
	}

	void bindDescriptorSets(VulkanDevice& inDevice, VkCommandBuffer inCommandBuffer, VkPipelineLayout inPipelineLayout, BufferHandle inFrameData, VkDescriptorSet inFrameBuffer, const MaterialProxy& inMaterialProxy, PipelineState& outPipelineState)
	{
		std::vector<uint32_t> dynamicOffsets;
		std::vector<VkDescriptorSet> descriptorSets;

		// Set 0 : Frame Data
		outPipelineState.frameDescriptorSet = inFrameBuffer;
		descriptorSets.push_back(inFrameBuffer);
		outPipelineState.frameBuffer = inFrameData;
		dynamicOffsets.push_back(inFrameData.offset);

		// Set 1 : Texture Data
		outPipelineState.textureDescriptorSet = inDevice.getTextureDescriptorSet(inMaterialProxy.getTextureDescriptorSetHandle());
		descriptorSets.push_back(outPipelineState.textureDescriptorSet);

		// Set 2 : Material Data
		outPipelineState.materialDescriptorSet = inDevice.getMaterialBufferDescriptorSet(inMaterialProxy.getMaterialDescriptorSetHandle());
		descriptorSets.push_back(outPipelineState.materialDescriptorSet);
		outPipelineState.materialBuffer = inMaterialProxy.getMaterialBufferHandle();
		dynamicOffsets.push_back(outPipelineState.materialBuffer.offset);

		vkCmdBindDescriptorSets(inCommandBuffer
			, VK_PIPELINE_BIND_POINT_GRAPHICS
			, inPipelineLayout
			, 0
			, (uint32_t)descriptorSets.size()
			, descriptorSets.data()
			, (uint32_t)dynamicOffsets.size()
			, dynamicOffsets.data());
	}

	void updateMaterialDescriptorSet(VulkanDevice& inDevice, VkCommandBuffer inCommandBuffer, VkPipelineLayout inPipelineLayout, const MaterialProxy& inMaterialProxy, PipelineState& outPipelineState)
	{
		std::vector<uint32_t> dynamicOffsets;
		std::vector<VkDescriptorSet> descriptorSets;

		// Set 2 : Material Data
		outPipelineState.materialDescriptorSet = inDevice.getMaterialBufferDescriptorSet(inMaterialProxy.getMaterialDescriptorSetHandle());
		descriptorSets.push_back(outPipelineState.materialDescriptorSet);
		outPipelineState.materialBuffer = inMaterialProxy.getMaterialBufferHandle();
		dynamicOffsets.push_back(outPipelineState.materialBuffer.offset);

		vkCmdBindDescriptorSets(inCommandBuffer
			, VK_PIPELINE_BIND_POINT_GRAPHICS
			, inPipelineLayout
			, 2
			, (uint32_t)descriptorSets.size()
			,descriptorSets.data()
			, (uint32_t)dynamicOffsets.size()
			, dynamicOffsets.data());
	}

	BufferHandle pushInstanceData(VulkanDevice& inDevice, VulkanBuffer& inBuffer, const glm::mat4& inModelMat)
	{
		InstanceData instanceData{};
		instanceData.model = inModelMat;

		return inDevice.pushFrameData((void*)&instanceData, sizeof(InstanceData), inBuffer);
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
	state = new PipelineState();
}

void RenderBackend::getContextRequirements(std::vector<std::pair<contextHints::Requirements, uint8_t>>& outHints)
{
	outHints.push_back({ contextHints::Requirements::ClientAPI, (uint8_t)contextHints::EClientAPI::NONE });
}

VulkanDevice& RenderBackend::getDevice()
{
	return impl->device;
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

	if (!impl->device.getGraphicsCommandQueue().createCommandBuffers(renderConfig::getFrameCount(), impl->graphicsCommandBuffers))
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

	uint32_t pipelineHandle = impl->device.createPipeline(impl->geometryRenderPass.getRenderPass()
		, impl->swapchain.getExtent()
		, shaderStageInfos
		, vertexInputAttributeDesc
		, vertexSize
		, descriptorSetLayouts
		, pushConstantRanges);

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

void RenderBackend::render(Scene& inScene)
{
	vkWaitForFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId], VK_TRUE, UINT64_MAX);

	uint32_t imageId;
	VkResult result = vkAcquireNextImageKHR(impl->device.getDevice(), impl->swapchain.get(), UINT64_MAX, impl->imageAvailableSemaphores[impl->currentFrameId], VK_NULL_HANDLE, &imageId);

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

	VkCommandBuffer commandBuffer = impl->graphicsCommandBuffers[impl->currentFrameId];
	vkResetCommandBuffer(commandBuffer, 0);

	uint8_t* commandData = (uint8_t*)render::getCommandRecorder()->getCommands().get(0);
	uint32_t commandSize = (uint32_t)render::getCommandRecorder()->getCommands().size();
	executeCommands(commandBuffer
		, commandData
		, commandSize
		, imageId
		, impl->currentFrameId);
	render::getCommandRecorder()->reset();
	state->reset();

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

	result = vkQueueSubmit(impl->device.getGraphicsCommandQueue().getQueue(), 1, &submitInfo, impl->inFlightFences[impl->currentFrameId]);
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
	presentInfo.pImageIndices = &imageId;
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

//
bool RenderBackend::executeCommands(VkCommandBuffer inCommandBuffer
	, uint8_t* inCommands
	, uint32_t inCommandsSize
	, uint32_t inImageId
	, uint32_t inFrameId)
{
	if (!beginCommandBuffer(inCommandBuffer))
	{
		return false;
	}

	VkDescriptorSet frameBufferDescriptorSet = impl->device.getFrameBufferDescriptorSet(inFrameId);
	VulkanBuffer& frameBuffer = impl->device.getBuffer(impl->device.getFrameBufferHandle(inFrameId));
	const BufferHandle frameDataHandle = pushFrameData(impl->device, frameBuffer);
	
	size_t offset = 0;
	while (offset < inCommandsSize)
	{
		CommandHeader* header = (CommandHeader*)(inCommands + offset);
		ensure(header);

		offset += sizeof(CommandHeader);
		switch (header->type)
		{
			case ECommandType::StartRenderpass:
			{
				StartRenderpassCommand* startRenderpassCommand = (StartRenderpassCommand*)(inCommands + offset);
				executeStartRenderpass(inCommandBuffer, startRenderpassCommand, inImageId);
				offset += sizeof(StartRenderpassCommand);
				break;
			}
			case ECommandType::EndRenderpass:
			{
				EndRenderpassCommand* endRenderpassCommand = (EndRenderpassCommand*)(inCommands + offset);
				executeEndRenderpass(inCommandBuffer, endRenderpassCommand);
				offset += sizeof(EndRenderpassCommand);
				break;
			}
			case ECommandType::BindMaterial:
			{
				BindMaterialCommand* bindMaterialCommand = (BindMaterialCommand*)(inCommands + offset);
				executeBindMaterial(inCommandBuffer, bindMaterialCommand, frameDataHandle, frameBufferDescriptorSet);
				offset += sizeof(BindMaterialCommand);
				break;
			}
			case ECommandType::DrawIndexed:
			{
				DrawIndexCommand* drawIndexCommand = (DrawIndexCommand*)(inCommands + offset);
				executeDrawIndexed(inCommandBuffer, drawIndexCommand, frameBuffer);
				offset += sizeof(DrawIndexCommand);
				break;
			}
		}
	}

	return endCommandBuffer(inCommandBuffer);
}

void RenderBackend::executeStartRenderpass(VkCommandBuffer inCommandBuffer, StartRenderpassCommand* inStartRenderpassCommand, uint8_t inImageIndex)
{
	// todo: retrieve renderpass
	VulkanGeometryRenderPass renderPass = impl->geometryRenderPass;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass.getRenderPass();
	renderPassInfo.framebuffer = renderPass.getFramebuffers()[inImageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = impl->swapchain.getExtent();

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0, 0, 0, 1 };
	clearValues[1].depthStencil = { 1, 0 };

	renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(inCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderBackend::executeEndRenderpass(VkCommandBuffer inCommandBuffer, EndRenderpassCommand* inEndRenderpassCommand)
{
	vkCmdEndRenderPass(inCommandBuffer);
}

void RenderBackend::executeBindMaterial(VkCommandBuffer inCommandBuffer, BindMaterialCommand* inBindMaterialCommand, BufferHandle inFrameBufferHandle, VkDescriptorSet inFrameBufferDescriptorSet)
{
	MaterialProxy& materialProxy = render::getRenderer()->getMaterialProxy(inBindMaterialCommand->material);
	
	// todo: get compatible pipeline
	uint32_t pipelineHandle = impl->device.getCompatiblePipeline(materialProxy);
	const VulkanPipeline* newPipeline = nullptr;
	if (state->pipelineHandle != pipelineHandle)
	{
		newPipeline = &impl->device.getPipeline(pipelineHandle);
		bindPipeline(inCommandBuffer, *newPipeline, impl->swapchain.getExtent());


		if (state->pipelineHandle == GPURESOURCE_INVALID_ID)
		{
			bindDescriptorSets(impl->device, inCommandBuffer, newPipeline->getPipelineLayout(), inFrameBufferHandle, inFrameBufferDescriptorSet, materialProxy, *state);
		}

		state->pipelineLayout = newPipeline->getPipelineLayout();
		state->pipelineHandle = pipelineHandle;
	}
	else
	{
		newPipeline = &impl->device.getPipeline(pipelineHandle);
	}

	updateMaterialDescriptorSet(impl->device, inCommandBuffer, state->pipelineLayout, materialProxy, *state);
}

void RenderBackend::executeDrawIndexed(VkCommandBuffer inCommandBuffer, DrawIndexCommand* inDrawIndexCommand, VulkanBuffer& inFrameBuffer)
{
	MaterialProxy& materialProxy = render::getRenderer()->getMaterialProxy(inDrawIndexCommand->material);
	MeshProxy& meshProxy = render::getRenderer()->getMeshProxy(inDrawIndexCommand->mesh);

	const VulkanBuffer& vertexBuffer = impl->device.getBuffer(meshProxy.getVertexBuffer());
	const VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer() };
	const VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(inCommandBuffer, 0, 1, vertexBuffers, offsets);

	const VulkanBuffer& indexBuffer = impl->device.getBuffer(meshProxy.getIndexBuffer());
	vkCmdBindIndexBuffer(inCommandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

	VkShaderStageFlags stageFlags = getShaderStageFlags(EShaderStageFlagBits::VERTEX);

	BufferHandle instanceDataHandle = pushInstanceData(impl->device, inFrameBuffer, inDrawIndexCommand->model);
	uint8_t* dataPtr = (uint8_t*)inFrameBuffer.getMappedMemory() + instanceDataHandle.offset;
	vkCmdPushConstants(inCommandBuffer, state->pipelineLayout, stageFlags, 0, instanceDataHandle.size, dataPtr);

	vkCmdDrawIndexed(inCommandBuffer, meshProxy.getIndexCount(), 1, 0, 0, 0);
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