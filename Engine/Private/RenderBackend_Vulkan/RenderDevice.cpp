#include "RenderBackend_Vulkan/RenderDevice.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "RenderBackend_Vulkan/CommandContext.h"
#include "RenderBackend_Vulkan/RenderBuffer.h"
#include "RenderBackend_Vulkan/RenderImage.h"
#include "RenderBackend_Vulkan/RenderSwapchain.h"

uint32_t getVkBufferType(EBufferType inType)
{
	switch (inType)
	{
	case EBufferType::VERTEX_ARRAY:
		ensure(false);
		return 0; // should not be needed
		break;
	case EBufferType::VERTICES:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case EBufferType::INDICES:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	}
	ensure(false);
	return 0;
}

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool RenderDevice::initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers, uint32_t inFrameCount)
{
	if (!pickPhysicalDevice(inInstance, inSurface, physicalDevice))
	{
		return false;
	}

	if (!createLogicalDevice(inInstance, inSurface, bInEnableValidationLayers, inValidationLayers))
	{
		return false;
	}

	std::vector<DescriptorPoolElement> descriptorPoolElements;

	DescriptorPoolElement uniformPoolElement;
	uniformPoolElement.count = inFrameCount;
	uniformPoolElement.type = DescriptorType::UNIFORM_BUFFER;
	descriptorPoolElements.push_back(uniformPoolElement);

	DescriptorPoolElement textureSamplerPoolElement;
	textureSamplerPoolElement.count = inFrameCount;
	textureSamplerPoolElement.type = DescriptorType::TEXTURE_SAMPLER;
	descriptorPoolElements.push_back(textureSamplerPoolElement);

	return createMainDescriptorPool(descriptorPoolElements, inFrameCount);
}

void RenderDevice::shutdown()
{
	for (size_t i = 0; i < commandContexts.size(); ++i)
	{
		commandContexts[i].free();
	}

	vkDestroyDescriptorPool(device, mainDescriptorPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

void RenderDevice::createCommandContexts(const QueueFamilyIndices& inQueueFamilyIndices)
{
	commandContexts.resize((uint8_t)EQueueFamily::COUNT);

	const uint32_t graphicsFamily = queueFamilyIndices.graphicsFamily.value();
	CommandContext& graphicsCommandContext = commandContexts[(uint8_t)EQueueFamily::GRAPHICS];
	graphicsCommandContext.create(device, graphicsFamily);

	const uint32_t transferFamily = queueFamilyIndices.transferFamily.value();
	CommandContext& transferCommandContext = commandContexts[(uint8_t)EQueueFamily::TRANSFER];
	transferCommandContext.create(device, transferFamily);

	const uint32_t presentFamily = queueFamilyIndices.presentFamily.value();
	CommandContext& presentCommandContext = commandContexts[(uint8_t)EQueueFamily::PRESENT];
	presentCommandContext.create(device, presentFamily);
}

GPUHandle RenderDevice::createAndFillBuffer(size_t inSize, void* inData, EBufferType inType)
{
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	GPUHandle stagingBufferHandle = createBuffer(inSize, usage, queueFamilyIndices, propertyFlags);
	if (!stagingBufferHandle.isValid())
	{
		return false;
	}

	if (!fillBuffer(stagingBufferHandle, inData, inSize))
	{
		return false;
	}

	usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | getVkBufferType(inType);
	propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUHandle vertexBufferHandle = createBuffer(inSize, usage, queueFamilyIndices, propertyFlags);
	if (!vertexBufferHandle.isValid())
	{
		return false;
	}

	const RenderBuffer& stagingBuffer = getBuffer(stagingBufferHandle);
	const RenderBuffer& vertexBuffer = getBuffer(vertexBufferHandle);
	CommandContext commandContext = getTransferCommandContext();
	if (!commandContext.isValid())
	{
		commandContext = getGraphicsCommandContext();
	}
	commandContext.executeCopyBufferCommand(stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), inSize);
	destroyBuffer(stagingBufferHandle);

	return vertexBufferHandle;
}

GPUHandle RenderDevice::createBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = inSize;
	bufferInfo.usage = inUsage;

	if (inQueueIndices.size() >= 2)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	}
	else
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	bufferInfo.queueFamilyIndexCount = (uint32_t)inQueueIndices.size();
	bufferInfo.pQueueFamilyIndices = inQueueIndices.data();

	VkBuffer buffer;
	VkDeviceMemory memory;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateBuffer Vertex failed" << std::endl;
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t memoryType = 0;
	RenderDevice::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, inProperties, memoryType);
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateMemory failed" << std::endl;
		return GPUHandle();
	}

	result = vkBindBufferMemory(device, buffer, memory, 0);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkBindBufferMemory failed" << std::endl;
		return GPUHandle();
	}

	renderBuffers.push_back(RenderBuffer(device, buffer, memory));
	return GPUHandle((uint32_t)(renderBuffers.size() - 1));
}

const RenderBuffer& RenderDevice::getBuffer(GPUHandle inHandle) const
{
	return renderBuffers.at(inHandle.id);
}

bool RenderDevice::fillBuffer(GPUHandle inHandle, void* inData, size_t inDataSize)
{
	const RenderBuffer& buffer = getBuffer(inHandle);

	void* data;
	vkMapMemory(buffer.getDevice(), buffer.getMemory(), 0, inDataSize, 0, &data);
	memcpy(data, inData, inDataSize);
	vkUnmapMemory(buffer.getDevice(), buffer.getMemory());
	return true;
}

void RenderDevice::destroyBuffer(GPUHandle handle)
{
	RenderBuffer& renderBuffer = renderBuffers.at(handle.id);
	vkDestroyBuffer(device, renderBuffer.getBuffer(), nullptr);
	vkFreeMemory(device, renderBuffer.getMemory(), nullptr);
}

GPUHandle RenderDevice::createImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties)
{
	VkImage image;
	VkDeviceMemory memory;

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
	}
	else
	{
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	imageInfo.queueFamilyIndexCount = (uint32_t)inQueueIndices.size();
	imageInfo.pQueueFamilyIndices = inQueueIndices.data();

	VkResult result = vkCreateImage(getDevice(), &imageInfo, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateImage failed" << std::endl;
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(getDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t memoryType;
	RenderDevice::findMemoryType(getPhysicalDevice(), memRequirements.memoryTypeBits, inProperties, memoryType);
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(getDevice(), &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateMemory failed" << std::endl;
		return false;
	}
	vkBindImageMemory(getDevice(), image, memory, 0);

	renderImages.push_back(RenderImage(device, image, memory));
	return GPUHandle((uint32_t)(renderImages.size() - 1));
}

RenderImage& RenderDevice::getImage(GPUHandle inHandle)
{
	return renderImages.at(inHandle.id);
}

const RenderImage& RenderDevice::getImage(GPUHandle inHandle) const
{
	return renderImages.at(inHandle.id);
}

void RenderDevice::destroyImage(GPUHandle inHandle)
{
	RenderImage& renderImage = renderImages.at(inHandle.id);

	vkDestroyImage(device, renderImage.getImage(), nullptr);
	vkDestroyImageView(device, renderImage.getView(), nullptr);
	vkFreeMemory(device, renderImage.getMemory(), nullptr);

	renderImage = RenderImage(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
}

bool RenderDevice::createMainDescriptorPool(const std::vector<DescriptorPoolElement>& inDescriptorPoolElements, uint32_t inMaxSets)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (size_t i = 0; i < inDescriptorPoolElements.size(); ++i)
	{
		const auto& poolElement = inDescriptorPoolElements[i];

		VkDescriptorPoolSize poolSize;
		poolSize.type = getDescriptorType(poolElement.type);
		poolSize.descriptorCount = poolElement.count;
		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = inMaxSets;
	VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &mainDescriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDescriptorPool failed" << std::endl;
		return false;
	}
	return true;
}

QueueFamilyIndices RenderDevice::findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface)
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

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT
			&& !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			indices.transferFamily = i;
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

bool RenderDevice::findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType)
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

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(inPhysicalDevice, &supportedFeatures);
	if (!supportedFeatures.geometryShader || !supportedFeatures.samplerAnisotropy)
	{
		return 0;
	}

	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	if (!supportedFeatures.geometryShader)
	{
		score += deviceProperties.limits.maxImageDimension2D;
	}

	return score;
}

bool RenderDevice::pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice)
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

	outPhysicalDevice = ratedDevices[0].second;
	return true;
}

bool RenderDevice::createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers)
{
	queueFamilyIndices = findQueueFamilies(physicalDevice, inSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	
	std::set<uint32_t> uniqueQueueFamilies = queueFamilyIndices.getIndicesSet();
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
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	createInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDevice failed" << std::endl;
		return false;
	}

	return true;
}