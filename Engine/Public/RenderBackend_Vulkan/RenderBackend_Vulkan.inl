#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/vulkan.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

#include "Core/Ensure.h"
#include "Core/FileSystem.h"
#include "Platform/Platform.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "Render/GraphicResourceHandle.h"
#include "RenderBackend/RenderBackend.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderInstance.h"
#include "RenderBackend_Vulkan/RenderPass.h"
#include "RenderBackend_Vulkan/RenderQueueFamily.h"
#include "RenderBackend_Vulkan/RenderSwapchain.h"

struct MyVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(MyVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(MyVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(MyVertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(MyVertex, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct RenderBackend::Impl
{
	VkSurfaceKHR surface;
	RenderInstance instance;

	RenderDevice device;
	RenderSwapchain swapchain;

	RenderPass geometryRenderPass;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	CommandContext graphicsCommandContext;
	CommandContext transferCommandContext;
	CommandContext presentCommandContext;

	// scene
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkDescriptorPool descriptorPool;
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
	bool createSurface(VkInstance inInstance, VkSurfaceKHR* inSurface)
	{
		WindowManager& windowManager = platform::getWindowManager();
		VkResult result = glfwCreateWindowSurface(inInstance, (GLFWwindow*)windowManager.getWindow(), nullptr, inSurface);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: createWindowSurface failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createShaderModule(VkDevice inDevice, const std::vector<char>& inCode, VkShaderModule& outShaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = inCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(inCode.data());

		VkResult result = vkCreateShaderModule(inDevice, &createInfo, nullptr, &outShaderModule);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateShaderModule failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createBuffer(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface, VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = inSize;
		bufferInfo.usage = inUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

		QueueFamilyIndices indices = RenderDevice::findQueueFamilies(inPhysicalDevice, inSurface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;

		VkResult result = vkCreateBuffer(inDevice, &bufferInfo, nullptr, &outBuffer);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateBuffer Vertex failed" << std::endl;
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(inDevice, outBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;

		uint32_t memoryType = 0;
		if (!RenderDevice::findMemoryType(inPhysicalDevice
			, memRequirements.memoryTypeBits
			, inProperties
			, memoryType))
		{
			return false;
		}
		allocInfo.memoryTypeIndex = memoryType;

		result = vkAllocateMemory(inDevice, &allocInfo, nullptr, &outMemory);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateMemory failed" << std::endl;
			return false;
		}

		result = vkBindBufferMemory(inDevice, outBuffer, outMemory, 0);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkBindBufferMemory failed" << std::endl;
			return false;
		}

		return true;
	}

	bool fillBuffer(VkDevice inDevice, VkBuffer inBuffer, VkDeviceMemory inMemory, void* inData, size_t inDataSize)
	{
		void* data;
		vkMapMemory(inDevice, inMemory, 0, inDataSize, 0, &data);
		memcpy(data, inData, inDataSize);
		vkUnmapMemory(inDevice, inMemory);
		return true;
	}

	bool createVertexBuffer(VkDevice inDevice
		, VkPhysicalDevice inPhysicalDevice
		, VkSurfaceKHR inSurface
		, CommandContext& inCommandContext
		, const std::vector<MyVertex>& inVertices
		, VkBuffer& outVertexBuffer
		, VkDeviceMemory& outVertexMemory)
	{
		size_t dataSize = sizeof(inVertices[0]) * inVertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if (!createBuffer(inDevice, inPhysicalDevice, inSurface, dataSize, usage, propertyFlags, stagingBuffer, stagingMemory))
		{
			return false;
		}

		void* data = (void*)inVertices.data();
		if (!fillBuffer(inDevice, stagingBuffer, stagingMemory, data, dataSize))
		{
			return false;
		}

		usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		if (!createBuffer(inDevice, inPhysicalDevice, inSurface, dataSize, usage, propertyFlags, outVertexBuffer, outVertexMemory))
		{
			return false;
		}

		inCommandContext.executeCopyBufferCommand(stagingBuffer, outVertexBuffer, dataSize);

		vkDestroyBuffer(inDevice, stagingBuffer, nullptr);
		vkFreeMemory(inDevice, stagingMemory, nullptr);

		return true;
	}

	bool createIndexBuffer(VkDevice inDevice
		, VkPhysicalDevice inPhysicalDevice
		, VkSurfaceKHR inSurface
		, CommandContext& inCommandContext
		, const std::vector<uint16_t>& inIndices
		, VkBuffer& outBuffer
		, VkDeviceMemory& outMemory)
	{
		VkDeviceSize bufferSize = sizeof(inIndices[0]) * inIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemoryBuffer;
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createBuffer(inDevice, inPhysicalDevice, inSurface, bufferSize, usage, propertyFlags, stagingBuffer, stagingMemoryBuffer);
		fillBuffer(inDevice, stagingBuffer, stagingMemoryBuffer, (void*)inIndices.data(), bufferSize);

		usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		createBuffer(inDevice, inPhysicalDevice, inSurface, bufferSize, usage, propertyFlags, outBuffer, outMemory);
		inCommandContext.executeCopyBufferCommand(stagingBuffer, outBuffer, bufferSize);

		vkDestroyBuffer(inDevice, stagingBuffer, nullptr);
		vkFreeMemory(inDevice, stagingMemoryBuffer, nullptr);

		return true;
	}

	bool createDescriptorSetLayout(VkDevice inDevice, VkDescriptorSetLayout& outDescriptorSetLayout)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)setLayoutBindings.size();
		layoutInfo.pBindings = setLayoutBindings.data();

		VkResult result = vkCreateDescriptorSetLayout(inDevice, &layoutInfo, nullptr, &outDescriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateDescriptorSetLayout failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createUniformBuffers(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface, std::vector<VkBuffer>& outBuffers, std::vector<VkDeviceMemory>& outMemories, std::vector<void*>& outMappedMemories)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		outBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		outMemories.resize(MAX_FRAMES_IN_FLIGHT);
		outMappedMemories.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			constexpr VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			createBuffer(inDevice, inPhysicalDevice, inSurface, bufferSize, usageFlags, memoryFlags, outBuffers[i], outMemories[i]);
			vkMapMemory(inDevice, outMemories[i], 0, bufferSize, 0, &outMappedMemories[i]);
		}

		return true;
	}

	bool createDescriptorPool(VkDevice inDevice, VkDescriptorPool& outDescriptorPool)
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT;

		VkResult result = vkCreateDescriptorPool(inDevice, &poolInfo, nullptr, &outDescriptorPool);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateDescriptorPool failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createDescriptorSets(VkDevice inDevice, VkDescriptorSetLayout inDescriptorSetLayout, VkDescriptorPool inDescriptorPool, const std::vector<VkBuffer>& inBuffers, VkImageView inTextureImageView, VkSampler inTextureSampler, std::vector<VkDescriptorSet>& outDescriptorSets)
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, inDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = inDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		outDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult result = vkAllocateDescriptorSets(inDevice, &allocInfo, outDescriptorSets.data());
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateDescriptorSets failed" << std::endl;
			return false;
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = inBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = inTextureImageView;
			imageInfo.sampler = inTextureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = outDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = outDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(inDevice, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		return true;
	}

	bool createGraphicsPipeline(VkDevice inDevice
		, VkExtent2D inSwapchainExtent
		, VkRenderPass inRenderPass
		, VkDescriptorSetLayout inDescriptorSetLayout
		, VkPipelineLayout& outPipelineLayout
		, VkPipeline& outGraphicsPipeline)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &inDescriptorSetLayout;

		VkResult result = vkCreatePipelineLayout(inDevice, &pipelineLayoutInfo, nullptr, &outPipelineLayout);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreatePipelineLayout failed" << std::endl;
			return false;
		}

		std::vector<char> vertShader;
		File::readFile(File::getProjectDir() + "Resources/Shaders/Vulkan/default.vert.spv", vertShader);
		VkShaderModule vertShaderModule;
		createShaderModule(inDevice, vertShader, vertShaderModule);

		std::vector<char> fragShader;
		File::readFile(File::getProjectDir() + "Resources/Shaders/Vulkan/default.frag.spv", fragShader);
		VkShaderModule fragShaderModule;
		createShaderModule(inDevice, fragShader, fragShaderModule);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = MyVertex::getBindingDescription();
		auto attributeDescriptions = MyVertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)inSwapchainExtent.width;
		viewport.height = (float)inSwapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = inSwapchainExtent;

		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = outPipelineLayout;
		pipelineInfo.renderPass = inRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		result = vkCreateGraphicsPipelines(inDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &outGraphicsPipeline);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateGraphicsPipeline failed" << std::endl;
			return false;
		}

		//
		vkDestroyShaderModule(inDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(inDevice, vertShaderModule, nullptr);

		return true;
	}
	
	bool createTextureImage(RenderDevice& inRenderDevice, VkSurfaceKHR inSurface, CommandContext& inGraphicsCommandCtx, CommandContext& inTransferCommandCtx, VkImage& outTextureImage, VkDeviceMemory& outTextureMemory)
	{
		VkDevice device = inRenderDevice.getDevice();
		VkPhysicalDevice physicalDevice = inRenderDevice.getPhysicalDevice();

		const std::string textureName = "D:/VestEngine/Resources/Textures/texture.jpg";
		int width, height, texChannels;
		stbi_uc* pixels = stbi_load(textureName.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;
		if (!pixels)
		{
			std::cerr << "Error: load \"" << textureName << "\" failed" << std::endl;
			return false;
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createBuffer(device, physicalDevice, inSurface, imageSize, usage, properties, stagingBuffer, stagingBufferMemory);
		fillBuffer(device, stagingBuffer, stagingBufferMemory, (void*)pixels, imageSize);

		stbi_image_free(pixels);

		QueueFamilyIndices indices = inRenderDevice.findQueueFamilies(inRenderDevice.getPhysicalDevice(), inSurface);
		
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		if (!RenderImage::createImage(inRenderDevice, width, height, format, tiling, usage, {indices.graphicsFamily.value(), indices.transferFamily.value()}, properties, outTextureImage, outTextureMemory))
		{
			return false;
		}

		VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		inTransferCommandCtx.executeTransitionImageLayoutCommand(outTextureImage, format, oldLayout, newLayout);
		inTransferCommandCtx.executeCopyBufferToImageCommand(stagingBuffer, outTextureImage, width, height);

		oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inGraphicsCommandCtx.executeTransitionImageLayoutCommand(outTextureImage, format, oldLayout, newLayout);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		return true;
	}

	bool createTextureImageView(VkDevice inDevice, VkImage inTextureImage, VkImageView& outImageView)
	{
		return RenderImage::createImageView(inDevice, inTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, outImageView);
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
		, VkPipeline inPipeline
		, RenderPass inRenderPass
		, VkExtent2D inSwapchainExtent
		, uint32_t imageIndex
		, VkBuffer inVertexBuffer
		, uint32_t inIndexCount
		, VkBuffer inIndexBuffer
		, VkPipelineLayout inPipelineLayout
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
		renderPassInfo.renderPass = inRenderPass.getPass();
		renderPassInfo.framebuffer = inRenderPass.getFramebuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = inSwapchainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0, 0, 0, 1 };
		clearValues[1].depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(inCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipeline);

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

			VkBuffer vertexBuffers[] = { inVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(inCommandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(inCommandBuffer, inIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inPipelineLayout, 0, 1, &inDescriptorSets[inCurrentFrame], 0, nullptr);

			vkCmdDrawIndexed(inCommandBuffer, inIndexCount, 1, 0, 0, 0);

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

	void recreateSwapchain(const RenderDevice& inDevice, VkSurfaceKHR inSurface, RenderSwapchain& inSwapchain, RenderPass& inRenderPass, CommandContext& inTransferCommandContext)
	{
		vkDeviceWaitIdle(inDevice.getDevice());

		inSwapchain.shutdown();
		inRenderPass.free();

		inSwapchain.initialize(inDevice, inSurface);
		inRenderPass.createDepthResources(inDevice, inSwapchain, inTransferCommandContext);
		inRenderPass.createFramebuffers(inSwapchain);
	}

	void updateUniformBuffer(VkExtent2D inExtent, void* inMapped)
	{
		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1), (float)platform::getTime() * glm::radians(90.0f), glm::vec3(0, 0, 1));
		ubo.view = glm::lookAt(glm::vec3(2,2,2), glm::vec3(0,0,0), glm::vec3(0,0,1));
		ubo.proj = glm::perspective(glm::radians(45.0f), inExtent.width / (float)inExtent.height, 0.1f, 10.0f);

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

	if (!createSurface(impl->instance.getInstance(), &impl->surface))
	{
		return false;
	}

	if (!impl->device.initialize(impl->instance.getInstance(), impl->surface, impl->instance.isValidationEnabled(), validationLayers))
	{
		return false;
	}

	if (!impl->swapchain.initialize(impl->device, impl->surface))
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

	if (!impl->geometryRenderPass.createDepthResources(impl->device, impl->swapchain, impl->graphicsCommandContext))
	{
		return false;
	}

	if (!impl->geometryRenderPass.createFramebuffers(impl->swapchain))
	{
		return false;
	}

	// pipeline building
	if (!createDescriptorSetLayout(impl->device.getDevice(), impl->descriptorSetLayout))
	{
		return false;
	}

	if (!createGraphicsPipeline(impl->device.getDevice(), impl->swapchain.getExtent(), impl->geometryRenderPass.getPass(), impl->descriptorSetLayout, impl->pipelineLayout, impl->graphicsPipeline))
	{
		return false;
	}

	// scene
	if (!createTextureImage(impl->device, impl->surface, impl->graphicsCommandContext, impl->transferCommandContext, impl->textureImage, impl->textureImageMemory))
	{
		return false;
	}

	if (!createTextureImageView(impl->device.getDevice(), impl->textureImage, impl->textureImageView))
	{
		return false;
	}

	if (!createTextureSampler(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->textureSampler))
	{
		return false;
	}

	if (!createVertexBuffer(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->transferCommandContext, impl->vertices, impl->vertexBuffer, impl->vertexBufferMemory))
	{
		return false;
	}

	if (!createIndexBuffer(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->transferCommandContext, impl->indices, impl->indexBuffer, impl->indexBufferMemory))
	{
		return false;
	}

	if (!createUniformBuffers(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->uniformBuffers, impl->uniformBuffersMemory, impl->uniformBuffersMapped))
	{
		return false;
	}

	if (!createDescriptorPool(impl->device.getDevice(), impl->descriptorPool))
	{
		return false;
	}

	if (!createDescriptorSets(impl->device.getDevice(), impl->descriptorSetLayout, impl->descriptorPool, impl->uniformBuffers, impl->textureImageView, impl->textureSampler, impl->descriptorSets))
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

	impl->swapchain.shutdown();

	vkDestroySampler(impl->device.getDevice(), impl->textureSampler, nullptr);
	vkDestroyImageView(impl->device.getDevice(), impl->textureImageView, nullptr);
	vkDestroyImage(impl->device.getDevice(), impl->textureImage, nullptr);
	vkFreeMemory(impl->device.getDevice(), impl->textureImageMemory, nullptr);

	vkDestroyBuffer(impl->device.getDevice(), impl->vertexBuffer, nullptr);
	vkFreeMemory(impl->device.getDevice(), impl->vertexBufferMemory, nullptr);
	vkDestroyBuffer(impl->device.getDevice(), impl->indexBuffer, nullptr);
	vkFreeMemory(impl->device.getDevice(), impl->indexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyBuffer(impl->device.getDevice(), impl->uniformBuffers[i], nullptr);
		vkFreeMemory(impl->device.getDevice(), impl->uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(impl->device.getDevice(), impl->descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(impl->device.getDevice(), impl->descriptorSetLayout, nullptr);

	vkDestroyPipeline(impl->device.getDevice(), impl->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(impl->device.getDevice(), impl->pipelineLayout, nullptr);

	impl->geometryRenderPass.free();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(impl->device.getDevice(), impl->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(impl->device.getDevice(), impl->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(impl->device.getDevice(), impl->inFlightFences[i], nullptr);
	}

	impl->graphicsCommandContext.free();
	impl->transferCommandContext.free();
	impl->presentCommandContext.free();

	impl->device.shutdown();

	vkDestroySurfaceKHR(impl->instance.getInstance(), impl->surface, nullptr);
	impl->instance.shutdown();
}

void RenderBackend::tracyGPUCollect()
{}

void RenderBackend::drawFrame()
{
	vkWaitForFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(impl->device.getDevice(), impl->swapchain.get(), UINT64_MAX, impl->imageAvailableSemaphores[impl->currentFrameId], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain(impl->device, impl->surface, impl->swapchain, impl->geometryRenderPass, impl->transferCommandContext);
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
	recordCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId], impl->graphicsPipeline, impl->geometryRenderPass, impl->swapchain.getExtent(), imageIndex, impl->vertexBuffer, (uint32_t)impl->indices.size(), impl->indexBuffer, impl->pipelineLayout, impl->descriptorSets, impl->currentFrameId);

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
		recreateSwapchain(impl->device, impl->surface, impl->swapchain, impl->geometryRenderPass, impl->transferCommandContext);
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


//
GPUHandle RenderBackend::createTexture(uint8_t* inData, size_t inWidth, size_t inHeight, ETextureFormat inInternalFormat)
{
	return {};
}

void RenderBackend::bindTexture(GPUHandle inHandle, std::string_view inType, size_t inIndex)
{
}

void RenderBackend::deleteTexture(GPUHandle inHandle)
{
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