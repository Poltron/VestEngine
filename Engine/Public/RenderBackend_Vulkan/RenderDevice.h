#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

class RenderDevice
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkQueue transferQueue = VK_NULL_HANDLE;

public:
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkDevice getDevice() { return device; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentQueue() { return presentQueue; }
	VkQueue getTransferQueue() { return transferQueue; }

	bool initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
	void shutdown();

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() 
				&& presentFamily.has_value()
				&& transferFamily.has_value();
		}
	};

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface);

private:
	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);
	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface);
	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
};