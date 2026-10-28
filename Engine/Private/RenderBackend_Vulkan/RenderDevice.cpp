#include "RenderBackend_Vulkan/RenderDevice.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "Core/Engine.h"
#include "Core/ResourcesManager.h"
#include "Core/ResourceHandle.h"
#include "Render/Resources/Texture.h"
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

bool RenderDevice::initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers)
{
	if (!pickPhysicalDevice(inInstance, inSurface, physicalDevice))
	{
		return false;
	}

	if (!createLogicalDevice(inInstance, inSurface, bInEnableValidationLayers, inValidationLayers))
	{
		return false;
	}

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	minUniformBufferOffsetAlignment = (uint32_t)physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

	DescriptorPoolElement textureSamplerPoolElement;
	textureSamplerPoolElement.count = 16;
	textureSamplerPoolElement.type = EDescriptorType::TEXTURE_SAMPLER;

	DescriptorPoolElement uniformBufferPoolElement;
	uniformBufferPoolElement.count = RenderDevice::getFrameCount();
	uniformBufferPoolElement.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	
	const uint8_t maxSets = textureSamplerPoolElement.count + uniformBufferPoolElement.count;
	if (!descriptorPool.initialize(*this, { textureSamplerPoolElement, uniformBufferPoolElement }, maxSets))
	{
		return false;
	}

	const uint8_t samplerCount = 1;
	if (!createTextureSamplers(samplerCount))
	{
		return false;
	}
	
	if (!createFrameDynamicUniformBuffers(RenderDevice::getFrameCount(), getFrameUniformBufferSize(), frameUniformBuffers, frameUniformBufferDescriptorSets))
	{
		return false;
	}

	if (!createMaterialDescriptorSetLayout())
	{
		return false;
	}

	return true;
}

void RenderDevice::shutdown()
{
	for (size_t i = 0; i < textureSamplers.size(); ++i)
	{
		vkDestroySampler(device, textureSamplers[i], nullptr);
	}

	for (size_t i = 0; i < frameUniformBuffers.size(); ++i)
	{
		destroyBuffer(frameUniformBuffers[i]);
	}

	descriptorPool.releaseDescriptorSet((uint32_t)frameUniformBufferDescriptorSets.size(), frameUniformBufferDescriptorSets.data());

	for (size_t i = 0; i < commandContexts.size(); ++i)
	{
		commandContexts[i].free();
	}

	descriptorPool.shutdown();

	vkDestroyDevice(device, nullptr);
}

void RenderDevice::createCommandContexts(const QueueFamilyIndices& inQueueFamilyIndices)
{
	commandContexts.resize((uint8_t)EQueueFamily::COUNT);

	const uint32_t graphicsFamily = queueFamilyIndices.graphicsFamily.value();
	CommandContext& graphicsCommandContext = commandContexts[(uint8_t)EQueueFamily::GRAPHICS];
	graphicsCommandContext.create(device, graphicsFamily);

	const uint32_t presentFamily = queueFamilyIndices.presentFamily.value();
	CommandContext& presentCommandContext = commandContexts[(uint8_t)EQueueFamily::PRESENT];
	presentCommandContext.create(device, presentFamily);

	const uint32_t transferFamily = queueFamilyIndices.transferFamily.value();
	CommandContext& transferCommandContext = commandContexts[(uint8_t)EQueueFamily::TRANSFER];
	transferCommandContext.create(device, transferFamily);
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

	RenderBuffer& stagingBuffer = getBuffer(stagingBufferHandle);
	stagingBuffer.mapAndFill(inData, inSize);

	usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | getVkBufferType(inType);
	propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUHandle vertexBufferHandle = createBuffer(inSize, usage, queueFamilyIndices, propertyFlags);
	if (!vertexBufferHandle.isValid())
	{
		return false;
	}

	const RenderBuffer& vertexBuffer = getBuffer(vertexBufferHandle);
	CommandContext commandContext = getTransferCommandContext();
	if (!commandContext.isValid())
	{
		commandContext = getGraphicsCommandContext();
	}

	// note: need to do this cause previous buffer allocation has underlying vector reallocating so previous ref is stale
	stagingBuffer = getBuffer(stagingBufferHandle);
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

	renderBuffers.push_back(RenderBuffer(device, buffer, memory, allocInfo.allocationSize));
	return GPUHandle((uint32_t)(renderBuffers.size() - 1));
}

RenderBuffer& RenderDevice::getBuffer(GPUHandle inHandle)
{
	return renderBuffers.at(inHandle.id);
}

const RenderBuffer& RenderDevice::getBuffer(GPUHandle inHandle) const
{
	return renderBuffers.at(inHandle.id);
}

void RenderDevice::destroyBuffer(GPUHandle handle)
{
	RenderBuffer& renderBuffer = renderBuffers.at(handle.id);
	vkDestroyBuffer(device, renderBuffer.getBuffer(), nullptr);
	vkFreeMemory(device, renderBuffer.getMemory(), nullptr);
}

namespace
{
	uint32_t getPaddedSize(uint32_t inSize, uint32_t inAlignment)
	{
		ensure(inAlignment > 0);
		return (inSize + inAlignment - 1) & ~(inAlignment - 1);
	}
}

void RenderDevice::resetFrameData()
{
	nextUniformBufferOffset = 0;
}

UBOHandle RenderDevice::emplaceFrameData(void* inData, uint32_t inSize, uint32_t inFrameId)
{
	RenderBuffer& buffer = getBuffer(frameUniformBuffers[inFrameId]);
	ensure(buffer.isMapped());
	ensure(buffer.getSize() >= nextUniformBufferOffset + inSize);
	
	buffer.fill(inData, inSize, nextUniformBufferOffset);
	
	UBOHandle handle(nextUniformBufferOffset, inSize);
	nextUniformBufferOffset += getPaddedSize(inSize, minUniformBufferOffsetAlignment);
	return handle;
}


bool RenderDevice::createMaterialDescriptorSetLayout()
{
	std::vector<DescriptorSetLayoutElement> layoutElements;

	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::TEXTURE_SAMPLER;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	layoutElements.push_back(element0);

	DescriptorSetLayoutElement element1;
	element1.binding = 1;
	element1.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element1.stage = EShaderStageFlagBits::FRAGMENT;
	element1.dstArrayElement = 0;
	layoutElements.push_back(element1);

	if (!RenderDescriptorPool::createDescriptorSetLayout(device, layoutElements, materialDescriptorSetLayout))
	{
		return false;
	}
	return true;
}

bool RenderDevice::createMaterialDescriptorSet(ResourceHandle inHandle)
{
	// note : c/c from createMaterialDescriptorSetLayout
	std::vector<DescriptorSetLayoutElement> layoutElements;

	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::TEXTURE_SAMPLER;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	layoutElements.push_back(element0);

	DescriptorSetLayoutElement element1;
	element1.binding = 1;
	element1.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element1.stage = EShaderStageFlagBits::FRAGMENT;
	element1.dstArrayElement = 0;
	layoutElements.push_back(element1);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(RenderDevice::getFrameCount(), materialDescriptorSetLayout);

	const Material* material = engine::getResources()->getMaterial(inHandle);
	const Texture* albedo = engine::getResources()->getTexture(material->getAlbedo());

	DescriptorSetLayoutElementValues layoutValues((uint32_t)layoutElements.size(), RenderDevice::getFrameCount());
	for (uint32_t i = 0; i < RenderDevice::getFrameCount(); ++i)
	{
		uint32_t frameIndexOffset = i * RenderDevice::getFrameCount();

		GPUHandle imageHandle;
		layoutValues.images[frameIndexOffset + 0] = { albedo->getHandle(), getTextureSampler(0)};
		layoutValues.buffers[frameIndexOffset + 1] = { getFrameUniformBufferHandle(0) };
	}

	std::vector<VkDescriptorSet> descriptorSets;
	descriptorPool.createDescriptorSets(*this, descriptorSetLayouts, layoutElements, layoutValues, RenderDevice::getFrameCount(), descriptorSets);
	materialDescriptorSets.push_back(descriptorSets[0]);

	return true;
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

bool RenderDevice::createFrameDynamicUniformBuffers(uint8_t inFrameCount
	, VkDeviceSize inBufferSize
	, std::vector<GPUHandle>& outBufferHandles
	, std::vector<VkDescriptorSet>& outBufferDescriptorSets)
{
	constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value(), indices.graphicsFamily.value() };
	constexpr VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	outBufferHandles.resize(inFrameCount);

	for (size_t i = 0; i < inFrameCount; ++i)
	{
		outBufferHandles[i] = createBuffer(inBufferSize, usageFlags, queueFamilyIndices, memoryFlags);

		RenderBuffer& buffer = getBuffer(outBufferHandles[i]);
		buffer.map(inBufferSize);
	}
	
	std::vector<DescriptorSetLayoutElement> layoutElements;

	DescriptorSetLayoutElement element1;
	element1.binding = 0;
	element1.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element1.stage = EShaderStageFlagBits::VERTEX;
	element1.dstArrayElement = 0;
	layoutElements.push_back(element1);

	if (!RenderDescriptorPool::createDescriptorSetLayout(device, layoutElements, frameUniformBufferLayout))
	{
		return false;
	}
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(RenderDevice::getFrameCount(), frameUniformBufferLayout);

	DescriptorSetLayoutElementValues layoutValues((uint32_t)layoutElements.size(), inFrameCount);
	layoutValues.buffers[0] = { outBufferHandles[0] };
	layoutValues.buffers[1] = { outBufferHandles[1] };

	if (!descriptorPool.createDescriptorSets(*this, descriptorSetLayouts, layoutElements, layoutValues, inFrameCount, frameUniformBufferDescriptorSets))
	{
		return false;
	}

	return true;
}

bool RenderDevice::createTextureSamplers(uint8_t inCount)
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
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	for (uint8_t i = 0; i < inCount; ++i)
	{
		VkSampler sampler;
		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateSampler failed" << std::endl;
			return false;
		}

		textureSamplers.push_back(sampler);
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