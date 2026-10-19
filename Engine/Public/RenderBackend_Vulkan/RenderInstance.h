#pragma once

#include <vector>
#include <vulkan/vulkan.h>

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
}

class RenderInstance
{
	VkInstance instance;
	std::vector<VkExtensionProperties> supportedExtensions;
	VkDebugUtilsMessengerEXT debugMessenger;
	
public:
	VkInstance getInstance() const { return instance; }
	const std::vector<VkExtensionProperties>& getSupportedExtensions() const { return supportedExtensions; }
	bool isValidationEnabled()
	{
#if NDEBUG
		return false;
#else
		return true;
#endif
	}

	bool initialize()
	{
		if (isValidationEnabled() && !checkValidationLayerSupport())
		{
			return false;
		}

		if (!createVkInstance(&instance, supportedExtensions))
		{
			return false;
		}

		if (isValidationEnabled())
		{
			if (!VkFunctionsEXT::createVkDebugMessenger(instance, &debugMessenger))
			{
				return false;
			}
		}

		return true;
	}

	void shutdown()
	{
		if (isValidationEnabled())
		{
			VkFunctionsEXT::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);
	}

private:
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

		if (isValidationEnabled())
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
		if (isValidationEnabled())
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			VkFunctionsEXT::populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
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
};