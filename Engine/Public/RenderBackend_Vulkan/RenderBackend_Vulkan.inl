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

struct MyVertex
{
	glm::vec2 pos;
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
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
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
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;

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

	std::vector<VkCommandBuffer> graphicsCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrameId = 0;

	bool bFramebufferResized = false;

	const std::vector<MyVertex> vertices =
	{
		{{-0.5f, -0.5f},	{1.0f, 0.0f, 0.0f},		{1.0f, 0.0f}},
		{{0.5f, -0.5f},		{0.0f, 1.0f, 0.0f},		{0.0f, 0.0f}},
		{{0.5f, 0.5f},		{0.0f, 0.0f, 1.0f},		{0.0f, 1.0f}},
		{{-0.5f, 0.5f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = 
	{
		0, 1, 2, 2, 3, 0
	};
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

	bool findMemoryType(VkPhysicalDevice& inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(inPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (inTypeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
			{
				outMemoryType = i;
				return true;
			}
		}

		std::cerr << "Error: findMemoryType failed" << std::endl;
		return false;
	}

	bool createBuffer(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface, VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = inSize;
		bufferInfo.usage = inUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

		RenderDevice::QueueFamilyIndices indices = RenderDevice::findQueueFamilies(inPhysicalDevice, inSurface);
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
		if (!findMemoryType(inPhysicalDevice
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

	VkCommandBuffer beginSingleTimeCommands(VkDevice inDevice, VkCommandPool inCommandPool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = inCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(inDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkDevice inDevice, VkCommandPool inCommandPool, VkCommandBuffer inCommandBuffer, VkQueue inQueue)
	{
		vkEndCommandBuffer(inCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &inCommandBuffer;

		vkQueueSubmit(inQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(inQueue);

		vkFreeCommandBuffers(inDevice, inCommandPool, 1, &inCommandBuffer);
	}

	void copyBuffer(VkDevice inDevice, VkCommandPool inTransferCommandPool, VkQueue inTransferQueue, VkBuffer inSrcBuffer, VkBuffer inDstBuffer, VkDeviceSize inSize)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(inDevice, inTransferCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = inSize;
		vkCmdCopyBuffer(commandBuffer, inSrcBuffer, inDstBuffer, 1, &copyRegion);

		endSingleTimeCommands(inDevice, inTransferCommandPool, commandBuffer, inTransferQueue);
	}

	void transitionImageLayout(VkDevice inDevice, VkCommandPool inCommandPool, VkQueue inQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(inDevice, inCommandPool);
		
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			std::cerr << "Error: transitionImageLayout failed" << std::endl;
			endSingleTimeCommands(inDevice, inCommandPool, commandBuffer, inQueue);
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

		endSingleTimeCommands(inDevice, inCommandPool, commandBuffer, inQueue);
	}

	void copyBufferToImage(VkDevice inDevice, VkCommandPool inCommandPool, VkQueue inQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(inDevice, inCommandPool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(inDevice, inCommandPool, commandBuffer, inQueue);
	}

	bool createVertexBuffer(VkDevice inDevice
		, VkPhysicalDevice inPhysicalDevice
		, VkSurfaceKHR inSurface
		, VkCommandPool inTransferCommandPool
		, VkQueue inTransferQueue
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

		copyBuffer(inDevice, inTransferCommandPool, inTransferQueue, stagingBuffer, outVertexBuffer, dataSize);

		vkDestroyBuffer(inDevice, stagingBuffer, nullptr);
		vkFreeMemory(inDevice, stagingMemory, nullptr);

		return true;
	}

	bool createIndexBuffer(VkDevice inDevice
		, VkPhysicalDevice inPhysicalDevice
		, VkSurfaceKHR inSurface
		, VkCommandPool inTransferCommandPool
		, VkQueue inTransferQueue
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
		copyBuffer(inDevice, inTransferCommandPool, inTransferQueue, stagingBuffer, outBuffer, bufferSize);

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
		, VkPhysicalDevice inPhysicalDevice
		, VkExtent2D inSwapchainExtent
		, VkRenderPass inRenderPass
		, VkDescriptorSetLayout inDescriptorSetLayout
		, VkPipelineLayout& outPipelineLayout
		, VkPipeline& outGraphicsPipeline
		, VkBuffer& outVertexBuffer
		, VkDeviceMemory& outVertexBufferMemory)
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

	bool createImage(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties, VkImage& outTextureImage, VkDeviceMemory& outTextureMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = (uint32_t)inWidth;
		imageInfo.extent.height = (uint32_t)inHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = inFormat;
		imageInfo.tiling = inTiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = inUsage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		
		if (inQueueIndices.size() >= 2)
		{
			imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			imageInfo.queueFamilyIndexCount = (uint32_t)inQueueIndices.size();
			imageInfo.pQueueFamilyIndices = inQueueIndices.data();
		}
		else
		{
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VkResult result = vkCreateImage(inDevice, &imageInfo, nullptr, &outTextureImage);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateImage failed" << std::endl;
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(inDevice, outTextureImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;

		uint32_t memoryType;
		findMemoryType(inPhysicalDevice, memRequirements.memoryTypeBits, inProperties, memoryType);
		allocInfo.memoryTypeIndex = memoryType;

		result = vkAllocateMemory(inDevice, &allocInfo, nullptr, &outTextureMemory);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkAllocateMemory failed" << std::endl;
			return false;
		}

		vkBindImageMemory(inDevice, outTextureImage, outTextureMemory, 0);
		return true;
	}
	
	bool createTextureImage(RenderDevice& inRenderDevice, VkCommandPool inGraphicsCommandPool, VkQueue inGraphicsQueue, VkCommandPool inTransferCommandPool, VkQueue inTransferQueue, VkSurfaceKHR inSurface, VkImage& outTextureImage, VkDeviceMemory& outTextureMemory)
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

		RenderDevice::QueueFamilyIndices indices = inRenderDevice.findQueueFamilies(inRenderDevice.getPhysicalDevice(), inSurface);
		
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		if (!createImage(inRenderDevice.getDevice(), inRenderDevice.getPhysicalDevice(), width, height, format, tiling, usage, {indices.graphicsFamily.value(), indices.transferFamily.value()}, properties, outTextureImage, outTextureMemory))
		{
			return false;
		}

		VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transitionImageLayout(device, inTransferCommandPool, inTransferQueue, outTextureImage, format, oldLayout, newLayout);
		copyBufferToImage(device, inTransferCommandPool, inTransferQueue, stagingBuffer, outTextureImage, width, height);

		oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		transitionImageLayout(device, inGraphicsCommandPool, inGraphicsQueue, outTextureImage, format, oldLayout, newLayout);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		return true;
	}

	bool createTextureImageView(VkDevice inDevice, VkImage inTextureImage, VkImageView& outImageView)
	{
		return RenderSwapchain::createImageView(inDevice, inTextureImage, VK_FORMAT_R8G8B8A8_SRGB, outImageView);
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

	bool createCommandPool(VkDevice inDevice, VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface, uint32_t inQueueFamily, VkCommandPool& outCommandPool)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = inQueueFamily;

		VkResult result = vkCreateCommandPool(inDevice, &poolInfo, nullptr, &outCommandPool);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateCommandPool failed" << std::endl;
			return false;
		}
		return true;
	}

	bool createCommandBuffers(VkDevice inDevice, VkCommandPool inGraphicsCommandPool, std::vector<VkCommandBuffer>& outCommandBuffers)
	{
		outCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = inGraphicsCommandPool;
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

	bool recordCommandBuffer(VkCommandBuffer inCommandBuffer
		, VkPipeline inPipeline
		, VkRenderPass inRenderPass
		, const std::vector<VkFramebuffer>& inFramebuffers
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
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		return false;
	}

	if (!createVkInstance(&impl->instance, impl->supportedExtensions))
	{
		return false;
	}

	if (enableValidationLayers)
	{
		if (!createVkDebugMessenger(impl->instance, &impl->debugMessenger))
		{
			return false;
		}
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

	if (!createDescriptorSetLayout(impl->device.getDevice(), impl->descriptorSetLayout))
	{
		return false;
	}

	if (!createGraphicsPipeline(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->swapchain.getExtent(), impl->renderPass, impl->descriptorSetLayout, impl->pipelineLayout, impl->graphicsPipeline, impl->vertexBuffer, impl->vertexBufferMemory))
	{
		return false;
	}

	if (!impl->swapchain.createFramebuffers(impl->device.getDevice(), impl->renderPass))
	{
		return false;
	}

	// Command pools
	RenderDevice::QueueFamilyIndices queueFamilyIndices = RenderDevice::findQueueFamilies(impl->device.getPhysicalDevice(), impl->surface);
	uint32_t graphicsFamily = queueFamilyIndices.graphicsFamily.value();
	if (!createCommandPool(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, graphicsFamily, impl->graphicsCommandPool))
	{
		return false;
	}

	uint32_t transferFamily = queueFamilyIndices.transferFamily.value();
	if (!createCommandPool(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, transferFamily, impl->transferCommandPool))
	{
		return false;
	}

	if (!createTextureImage(impl->device, impl->graphicsCommandPool, impl->device.getGraphicsQueue(), impl->transferCommandPool, impl->device.getTransferQueue(), impl->surface, impl->textureImage, impl->textureImageMemory))
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

	if (!createVertexBuffer(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->transferCommandPool, impl->device.getTransferQueue(), impl->vertices, impl->vertexBuffer, impl->vertexBufferMemory))
	{
		return false;
	}

	if (!createIndexBuffer(impl->device.getDevice(), impl->device.getPhysicalDevice(), impl->surface, impl->transferCommandPool, impl->device.getTransferQueue(), impl->indices, impl->indexBuffer, impl->indexBufferMemory))
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
	if (!createCommandBuffers(impl->device.getDevice(), impl->graphicsCommandPool, impl->graphicsCommandBuffers))
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

	vkDestroyRenderPass(impl->device.getDevice(), impl->renderPass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(impl->device.getDevice(), impl->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(impl->device.getDevice(), impl->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(impl->device.getDevice(), impl->inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(impl->device.getDevice(), impl->graphicsCommandPool, nullptr);
	vkDestroyCommandPool(impl->device.getDevice(), impl->transferCommandPool, nullptr);

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

	updateUniformBuffer(impl->swapchain.getExtent(), impl->uniformBuffersMapped[impl->currentFrameId]);

	vkResetCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId], 0);
	recordCommandBuffer(impl->graphicsCommandBuffers[impl->currentFrameId], impl->graphicsPipeline, impl->renderPass, impl->swapchain.getFramebuffers(), impl->swapchain.getExtent(), imageIndex, impl->vertexBuffer, (uint32_t)impl->indices.size(), impl->indexBuffer, impl->pipelineLayout, impl->descriptorSets, impl->currentFrameId);

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