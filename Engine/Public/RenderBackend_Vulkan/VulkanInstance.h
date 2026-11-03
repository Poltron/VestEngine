#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class VulkanInstance
{
	VkInstance instance;
	std::vector<VkExtensionProperties> supportedExtensions;
	VkDebugUtilsMessengerEXT debugMessenger;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

public:
	VkInstance getInstance() const { return instance; }
	const std::vector<VkExtensionProperties>& getSupportedExtensions() const { return supportedExtensions; }

	bool isValidationEnabled() const
	{
#if NDEBUG
		return false;
#else
		return true;
#endif
	}

	bool initialize();
	void shutdown();

private:
	bool createVkInstance(VkInstance* outVkInstance, std::vector<VkExtensionProperties>& outVkExtensions);

	void getRequiredExtensions(std::vector<const char*>& outExtensions);
	void checkWindowRequiredExtensions(const std::vector<const char*>& inRequiredExtensions, const std::vector<VkExtensionProperties>& inSupportedExtensions);
	bool checkValidationLayerSupport();
};