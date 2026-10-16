#pragma once

#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

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

	std::set<uint32_t> getIndicesSet()
	{ 
		return std::set<uint32_t>({ graphicsFamily.value(), presentFamily.value(), transferFamily.value() });
	}
};

class RenderDevice
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices queueFamilyIndices;

public:
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	VkDevice getDevice() const { return device; }
	QueueFamilyIndices getQueueFamilyIndices() const { return queueFamilyIndices; }

	bool initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
	void shutdown();

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface);
	static bool findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType);

private:
	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);
	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice);
	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
};