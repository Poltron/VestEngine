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
#include "RenderBackend_Vulkan/RenderSwapchain.h"

#if NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

namespace VkFunctionsEXT
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}

struct RenderBackend::Impl
{
	VkInstance instance;
	std::vector<VkExtensionProperties> supportedExtensions;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	RenderDevice device;
	RenderSwapchain swapchain;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;

	uint32_t currentFrameId = 0;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	bool bFramebufferResized = false;
};



namespace
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		std::cerr << "Error message: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo)
	{
		outCreateInfo = {};
		outCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		outCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		outCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		outCreateInfo.pfnUserCallback = debugCallback;
		outCreateInfo.pUserData = render::getRenderBackend();
	}

	bool createVkDebugMessenger(VkInstance inInstance, VkDebugUtilsMessengerEXT* inDebugMessenger)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		VkResult result = VkFunctionsEXT::CreateDebugUtilsMessengerEXT(inInstance, &createInfo, nullptr, inDebugMessenger);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: Debug Messenger setup failed" << std::endl;
			return false;
		}

		return true;
	}

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

	void getRequiredExtensions(std::vector<const char*>& outExtensions)
	{
		uint32_t extensionCount = 0;
		const char** extensions = nullptr;
		platform::getWindowManager().getRequiredInstanceExtensions(&extensionCount, extensions);

		outExtensions.clear();
		for (size_t i = 0; i < extensionCount; ++i)
		{
			outExtensions.push_back(extensions[i]);
		}

		if (enableValidationLayers)
		{
			outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
	}

	void checkWindowRequiredExtensions(const std::vector<const char*>& inRequiredExtensions, const std::vector<VkExtensionProperties>& inSupportedExtensions)
	{
		std::cout << "Windowing required extensions : " << std::endl;
		for (uint32_t i = 0; i < inRequiredExtensions.size(); ++i)
		{
			std::cout << inRequiredExtensions[i];
		}
		std::cout << std::endl;

		std::cout << "Vulkan supported extensions : " << std::endl;
		for (size_t i = 0; i < inSupportedExtensions.size(); ++i)
		{
			std::cout << inSupportedExtensions[i].extensionName << " v." << inSupportedExtensions[i].specVersion;
		}
		std::cout << std::endl;

		bool bMissingExtensions = false;
		std::stringstream ss;
		for (uint32_t i = 0; i < inRequiredExtensions.size(); ++i)
		{
			bool bExtensionSupported = false;
			for (size_t j = 0; j < inSupportedExtensions.size(); ++j)
			{
				if (strcmp(inRequiredExtensions[i], inSupportedExtensions[j].extensionName) == 0)
				{
					bExtensionSupported = true;
					break;
				}
			}

			if (!bExtensionSupported)
			{
				bMissingExtensions = true;
				ss << inRequiredExtensions[i] << " ";
			}
		}

		if (bMissingExtensions)
		{
			std::cout << "ERROR: Required extensions missing: " << ss.str();
		}
	}

	bool createVkInstance(VkInstance* outVkInstance, std::vector<VkExtensionProperties>& outVkExtensions)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vest Editor";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Vest Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
			createInfo.pNext = &debugMessengerCreateInfo;
		}

		std::vector<const char*> requiredExtensions;
		getRequiredExtensions(requiredExtensions);

		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, outVkInstance);
		if (result != VK_SUCCESS)
		{
			// todo: debug doesn't work supportedExtensions is always empty
			outVkExtensions.resize(20);
			uint32_t supportedExtensionCount = 0; // todo: cleanup useless ?
			vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, outVkExtensions.data());
			checkWindowRequiredExtensions(requiredExtensions, outVkExtensions);

			std::cerr << "VkCreateInstance failed" << std::endl;
			return false;
		}

		return true;
	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::stringstream missingLayersSS;
		for (const char* layerName : validationLayers) 
		{
			bool bLayerFound = false;

			for (const auto& layerProperties : availableLayers) 
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) 
				{
					bLayerFound = true;
					break;
				}
			}

			if (!bLayerFound)
			{
				missingLayersSS << layerName << " ";
				return false;
			}
		}

		std::string missingLayers = missingLayersSS.str();
		if (missingLayers.size() == 0)
		{
			return true;
		}

		std::cerr << "ERROR: Missing validation layers: " << missingLayers << std::endl;
		return false;
	}

	bool createImageViews(RenderDevice& inDevice, RenderSwapchain& inSwapchain, std::vector<VkImageView>& outImageViews)
	{
		outImageViews.resize(inSwapchain.getImages().size());
		for (size_t i = 0; i < inSwapchain.getImages().size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = inSwapchain.getImages()[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = inSwapchain.getImageFormat();
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result = vkCreateImageView(inDevice.getDevice(), &createInfo, nullptr, &outImageViews[i]);
			if (result != VK_SUCCESS)
			{
				return false;
			}
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

	bool createRenderPass(VkDevice inDevice, VkFormat inFormat, VkRenderPass& outRenderPass)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = inFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(inDevice, &renderPassInfo, nullptr, &outRenderPass);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateRenderPass failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createGraphicsPipeline(VkDevice inDevice, VkExtent2D inSwapchainExtent, VkRenderPass inRenderPass, VkPipelineLayout& outPipelineLayout, VkPipeline& outGraphicsPipeline)
	{
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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
		
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
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		VkResult result = vkCreatePipelineLayout(inDevice, &pipelineLayoutInfo, nullptr, &outPipelineLayout);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreatePipelineLayout failed" << std::endl;
			return false;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
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

	bool createCommandPool(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface, VkCommandPool& outCommandPool)
	{
		RenderDevice::QueueFamilyIndices queueFamilyIndices = RenderDevice::findQueueFamilies(inPhysicalDevice, inSurface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VkResult result = vkCreateCommandPool(inDevice, &poolInfo, nullptr, &outCommandPool);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateCommandPool failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createCommandBuffers(VkDevice inDevice, VkCommandPool inCommandPool, std::vector<VkCommandBuffer>& outCommandBuffers)
	{
		outCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = inCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)outCommandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(inDevice, &allocInfo, outCommandBuffers.data());
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateCommandBuffers failed" << std::endl;
			return false;
		}
		return true;
	}

	bool recordCommandBuffer(VkCommandBuffer inCommandBuffer, VkPipeline inPipeline, VkRenderPass inRenderPass, const std::vector<VkFramebuffer>& inFramebuffers, VkExtent2D inSwapchainExtent, uint32_t imageIndex)
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
		renderPassInfo.renderPass = inRenderPass;
		renderPassInfo.framebuffer = inFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = inSwapchainExtent;

		VkClearValue clearColor = { 0,0,0,1 };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

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

		vkCmdDraw(inCommandBuffer, 3, 1, 0, 0);

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

	void destroySwapchain(VkDevice inDevice, RenderSwapchain inSwapchain)
	{
		for (auto framebuffer : inSwapchain.getFramebuffers()) {
			vkDestroyFramebuffer(inDevice, framebuffer, nullptr);
		}

		for (auto imageView : inSwapchain.getImageViews()) {
			vkDestroyImageView(inDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(inDevice, inSwapchain.get(), nullptr);
	}

	void recreateSwapchain(RenderDevice& inDevice, VkSurfaceKHR inSurface, RenderSwapchain& inSwapchain, VkRenderPass inRenderPass)
	{
		vkDeviceWaitIdle(inDevice.getDevice());

		inSwapchain.shutdown(inDevice);

		inSwapchain.initialize(inDevice, inSurface);
		inSwapchain.createImageViews(inDevice.getPhysicalDevice(), inDevice.getDevice(), inSurface);
		inSwapchain.createFramebuffers(inDevice.getDevice(), inRenderPass);
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
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		return false;
	}

	if (!createVkInstance(&impl->instance, impl->supportedExtensions))
	{
		return false;
	}

	if (!createVkDebugMessenger(impl->instance, &impl->debugMessenger))
	{
		return false;
	}

	if (!createSurface(impl->instance, &impl->surface))
	{
		return false;
	}

	if (!impl->device.initialize(impl->instance, impl->surface, enableValidationLayers, validationLayers))
	{
		return false;
	}

	if (!impl->swapchain.initialize(impl->device, impl->surface))
	{
		return false;
	}

	if (!impl->swapchain.createImageViews(impl->device.getPhysicalDevice(), impl->device.getDevice(), impl->surface))
	{
		return false;
	}

	if (!createRenderPass(impl->device.getDevice(), impl->swapchain.getImageFormat(), impl->renderPass))
	{
		return false;
	}

	if (!createGraphicsPipeline(impl->device.getDevice(), impl->swapchain.getExtent(), impl->renderPass, impl->pipelineLayout, impl->graphicsPipeline))
	{
		return false;
	}

	if (!impl->swapchain.createFramebuffers(impl->device.getDevice(), impl->renderPass))
	{
		return false;
	}

	if (!createCommandPool(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->commandPool))
	{
		return false;
	}

	if (!createCommandBuffers(impl->device.getDevice(), impl->commandPool, impl->commandBuffers))
	{
		return false;
	}

	if (!recordCommandBuffer(impl->commandBuffers[0], impl->graphicsPipeline, impl->renderPass, impl->swapchain.getFramebuffers(), impl->swapchain.getExtent(), 0))
	{
		return false;
	}

	if (!createSyncObjects(impl->device.getDevice(), impl->imageAvailableSemaphores, impl->renderFinishedSemaphores, impl->inFlightFences))
	{
		return false;
	}

	return true;
}

void RenderBackend::shutdown()
{
	vkDeviceWaitIdle(impl->device.getDevice());

	impl->swapchain.shutdown(impl->device);

	vkDestroyPipeline(impl->device.getDevice(), impl->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(impl->device.getDevice(), impl->pipelineLayout, nullptr);

	vkDestroyRenderPass(impl->device.getDevice(), impl->renderPass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(impl->device.getDevice(), impl->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(impl->device.getDevice(), impl->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(impl->device.getDevice(), impl->inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(impl->device.getDevice(), impl->commandPool, nullptr);

	impl->device.shutdown();

	if (enableValidationLayers)
	{
		VkFunctionsEXT::DestroyDebugUtilsMessengerEXT(impl->instance, impl->debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(impl->instance, impl->surface, nullptr);
	vkDestroyInstance(impl->instance, nullptr);
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
		recreateSwapchain(impl->device, impl->surface, impl->swapchain, impl->renderPass);
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cerr << "Error: vkAcquiredNextImageKHR failed" << std::endl;
		return;
	}

	vkResetFences(impl->device.getDevice(), 1, &impl->inFlightFences[impl->currentFrameId]);

	vkResetCommandBuffer(impl->commandBuffers[impl->currentFrameId], 0);
	recordCommandBuffer(impl->commandBuffers[impl->currentFrameId], impl->graphicsPipeline, impl->renderPass, impl->swapchain.getFramebuffers(), impl->swapchain.getExtent(), imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { impl->imageAvailableSemaphores[impl->currentFrameId] };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &impl->commandBuffers[impl->currentFrameId];

	VkSemaphore signalSemaphores[] = { impl->renderFinishedSemaphores[impl->currentFrameId] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = vkQueueSubmit(impl->device.getGraphicsQueue(), 1, &submitInfo, impl->inFlightFences[impl->currentFrameId]);
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

	vkQueuePresentKHR(impl->device.getPresentQueue(), &presentInfo);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || impl->bFramebufferResized)
	{
		impl->bFramebufferResized = false;
		recreateSwapchain(impl->device, impl->surface, impl->swapchain, impl->renderPass);
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