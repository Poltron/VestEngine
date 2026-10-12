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
#include "Platform/Platform.h"
#include "Platform/WindowHints.h"
#include "Platform/WindowManager.h"
#include "RenderBackend/RenderBackend.h"
#include "Render/GraphicResourceHandle.h"

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#if NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

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

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
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

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() 
				&& presentFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(inDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(inDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			const auto& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(inDevice, i, inSurface, &presentSupport);
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(inPhysicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(inPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface) 
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

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& inAvailableFormats)
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

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& inAvailablePresentModes)
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

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& inCapabilities)
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

	bool createSwapchain(VkPhysicalDevice inPhysicalDevice
		, VkDevice inDevice
		, VkSurfaceKHR inSurface
		, VkSwapchainKHR& outSwapchain
		, std::vector<VkImage>& outSwapchainImages
		, VkFormat& outSwapchainImageFormat
		, VkExtent2D& outSwapchainExtent)
	{
		SwapchainSupportDetails swapchainSupport = querySwapchainSupport(inPhysicalDevice, inSurface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		outSwapchainImageFormat = surfaceFormat.format;
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
		outSwapchainExtent = chooseSwapExtent(swapchainSupport.capabilities);

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
		swapchainCreateInfo.imageExtent = outSwapchainExtent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(inPhysicalDevice, inSurface);
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

		VkResult result = vkCreateSwapchainKHR(inDevice, &swapchainCreateInfo, nullptr, &outSwapchain);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateSwapchain failed" << std::endl;
			return false;
		}

		vkGetSwapchainImagesKHR(inDevice, outSwapchain, &imageCount, nullptr);
		outSwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(inDevice, outSwapchain, &imageCount, outSwapchainImages.data());

		return true;
	}

	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface)
	{
		QueueFamilyIndices indices = findQueueFamilies(inPhysicalDevice, inSurface);
		if (!indices.isComplete())
		{
			return 0;
		}

		bool bExtensionsSupported = checkDeviceExtensionSupport(inPhysicalDevice);
		if (!bExtensionsSupported)
		{
			return 0;
		}

		SwapchainSupportDetails swapchainDetails = querySwapchainSupport(inPhysicalDevice, inSurface);
		if (!swapchainDetails.formats.empty () && !swapchainDetails.presentModes.empty())
		{
			return 0;
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(inPhysicalDevice, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(inPhysicalDevice, &deviceFeatures);

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		int score = 0;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		if (!deviceFeatures.geometryShader)
		{
			score += deviceProperties.limits.maxImageDimension2D;
		}

		return score;
	}

	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outDevice)
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(inInstance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			std::cerr << "Error: No physical devices" << std::endl;
			return false;
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(inInstance, &physicalDeviceCount, physicalDevices.data());

		std::vector<std::pair<int, VkPhysicalDevice>> ratedDevices;
		for (const auto& physicalDevice : physicalDevices)
		{
			int score = ratePhysicalDevice(physicalDevice, inSurface);
			if (score > 0)
			{
				ratedDevices.push_back({ score, physicalDevice });
			}
		}

		if (ratedDevices.size() == 0)
		{
			std::cerr << "Error: No compatible physical device" << std::endl;
			return false;
		}

		std::sort(ratedDevices.begin(), ratedDevices.end()
			, [](const auto& inA, const auto& inB)
			{
				return inA.first < inB.first;
			});

		outDevice = ratedDevices[0].second;
		return true;
	}

	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice inPhysicalDevice, VkDevice& outDevice, VkQueue& outGraphicsQueue, VkQueue& outPresentQueue)
	{
		QueueFamilyIndices indices = findQueueFamilies(inPhysicalDevice, inSurface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		VkPhysicalDeviceFeatures deviceFeatures{};
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		VkResult result = vkCreateDevice(inPhysicalDevice, &createInfo, nullptr, &outDevice);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateDevice failed" << std::endl;
			return false;
		}

		vkGetDeviceQueue(outDevice, indices.graphicsFamily.value(), 0, &outGraphicsQueue);
		vkGetDeviceQueue(outDevice, indices.presentFamily.value(), 0, &outPresentQueue);
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

bool RenderBackend::initWindowGraphicLibrary(void* inProcAddress, size_t inWidth, size_t inHeight)
{
	return true;
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

	if (!pickPhysicalDevice(impl->instance, impl->surface, impl->physicalDevice))
	{
		return false;
	}

	if (!createLogicalDevice(impl->instance, impl->surface, impl->physicalDevice, impl->device, impl->graphicsQueue, impl->presentQueue))
	{
		return false;
	}

	if (!createSwapchain(impl->physicalDevice
		, impl->device
		, impl->surface
		, impl->swapchain
		, impl->swapchainImages
		, impl->swapchainImageFormat
		, impl->swapchainExtent))
	{
		return false;
	}

	return true;
}

void RenderBackend::shutdown()
{
	vkDestroyDevice(impl->device, nullptr);
	vkDestroySurfaceKHR(impl->instance, impl->surface, nullptr);

	if (enableValidationLayers)
	{
		VkFunctionsEXT::DestroyDebugUtilsMessengerEXT(impl->instance, impl->debugMessenger, nullptr);
	}

	vkDestroyInstance(impl->instance, nullptr);
}

void RenderBackend::tracyGPUCollect()
{
}

void RenderBackend::clear()
{
	ZoneScopedN("Renderer::clear");
}

void RenderBackend::resizeViewport(int inX, int inY, int inWidth, int inHeight)
{
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