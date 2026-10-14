#include "RenderBackend_Vulkan/RenderDevice.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "RenderBackend_Vulkan/RenderSwapchain.h"

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool RenderDevice::initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers)
{
	if (!pickPhysicalDevice(inInstance, inSurface))
	{
		return false;
	}
	return createLogicalDevice(inInstance, inSurface, bInEnableValidationLayers, inValidationLayers);
}

void RenderDevice::shutdown()
{
	vkDestroyDevice(device, nullptr);
}

RenderDevice::QueueFamilyIndices RenderDevice::findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface)
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
		if (presentSupport)
		{
			indices.presentFamily = i;
		}
	}

	return indices;
}

bool RenderDevice::checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice)
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

int RenderDevice::ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface)
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

	RenderSwapchain::SwapchainSupportDetails swapchainDetails = RenderSwapchain::querySwapchainSupport(inPhysicalDevice, inSurface);
	if (swapchainDetails.formats.empty() || swapchainDetails.presentModes.empty())
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

bool RenderDevice::pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface)
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

	physicalDevice = ratedDevices[0].second;
	return true;
}

bool RenderDevice::createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, inSurface);

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


	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDevice failed" << std::endl;
		return false;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	return true;
}