#include "RenderBackend_Vulkan/VulkanDevice.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "Core/Engine.h"
#include "Core/ResourcesManager.h"
#include "Core/ResourceHandle.h"
#include "Core/Resources/Texture.h"
#include "Render/Renderer.h"
#include "Render/Resources/MaterialProxy.h"
#include "Render/Resources/TextureProxy.h"
#include "Render/ShaderStructs.h"
#include "RenderBackend_Vulkan/VulkanCommandQueue.h"
#include "RenderBackend_Vulkan/VulkanBuffer.h"
#include "RenderBackend_Vulkan/VulkanConfig.h"
#include "RenderBackend_Vulkan/VulkanImage.h"
#include "RenderBackend_Vulkan/VulkanInstance.h"
#include "RenderBackend_Vulkan/VulkanPipeline.h"
#include "RenderBackend_Vulkan/VulkanSwapchain.h"
#include "RenderBackend_Vulkan/VulkanShaderHelper.h"

namespace
{
	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
	};

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
}


VulkanCommandQueue& VulkanDevice::getGraphicsCommandQueue()
{ return VulkanCommandQueues[(uint8_t)EQueueFamily::GRAPHICS]; }
VulkanCommandQueue& VulkanDevice::getPresentVulkanCommandQueue()
{ return VulkanCommandQueues[(uint8_t)EQueueFamily::PRESENT]; }
VulkanCommandQueue& VulkanDevice::getTransferCommandQueue()
{ return VulkanCommandQueues[(uint8_t)EQueueFamily::TRANSFER]; }
const VulkanPipeline& VulkanDevice::getPipeline(uint32_t inHandle) const
{ return pipelines[inHandle]; }

bool VulkanDevice::initialize(const VulkanInstance& inInstance, VkSurfaceKHR inSurface)
{
	if (!pickPhysicalDevice(inInstance.getInstance(), inSurface, physicalDevice))
	{
		return false;
	}

	if (!createLogicalDevice(inInstance.getInstance(), inSurface))
	{
		return false;
	}

	createVulkanCommandQueues(queueFamilyIndices);

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	minUniformBufferOffsetAlignment = (uint32_t)physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	maxUniformBufferRange = (uint32_t)physicalDeviceProperties.limits.maxUniformBufferRange;

	DescriptorPoolElement uniformBufferDescriptorSetPoolElement;
	uniformBufferDescriptorSetPoolElement.count = 4;
	uniformBufferDescriptorSetPoolElement.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;

	DescriptorPoolElement samplerDescriptorSetPoolElement;
	samplerDescriptorSetPoolElement.count = 1;
	samplerDescriptorSetPoolElement.type = EDescriptorType::SAMPLER;

	DescriptorPoolElement textureDescriptorSetPoolElement;
	textureDescriptorSetPoolElement.count = 16;
	textureDescriptorSetPoolElement.type = EDescriptorType::TEXTURE;
	
	const uint8_t maxSets = uniformBufferDescriptorSetPoolElement.count + samplerDescriptorSetPoolElement.count + textureDescriptorSetPoolElement.count;
	if (!descriptorPool.initialize(*this, { uniformBufferDescriptorSetPoolElement, samplerDescriptorSetPoolElement, textureDescriptorSetPoolElement }, maxSets))
	{
		return false;
	}
	
	if (!createFrameData(renderConfig::getFrameCount(), renderConfig::getFrameBufferSize(), frameUniformBuffers, frameUniformBufferDescriptorSets))
	{
		return false;
	}

	textureDescriptorSets.resize(1);
	if (!createTextureData(textureDescriptorSets[0]))
	{
		return false;
	}

	materialBuffers.resize(1);
	materialDescriptorSets.resize(1);
	if (!createMaterialData(renderConfig::getMaterialBufferSize(), materialBuffers[0], materialDescriptorSets[0]))
	{
		return false;
	}

	return true;
}

void VulkanDevice::shutdown()
{
	vkDestroySampler(device, sampler, nullptr);

	for (size_t i = 0; i < frameUniformBuffers.size(); ++i)
	{
		destroyBuffer(frameUniformBuffers[i]);
	}

	for (size_t i = 0; i < pipelines.size(); ++i)
	{
		pipelines[i].destroy();
	}

	descriptorPool.releaseDescriptorSet((uint32_t)frameUniformBufferDescriptorSets.size(), frameUniformBufferDescriptorSets.data());

	for (size_t i = 0; i < VulkanCommandQueues.size(); ++i)
	{
		VulkanCommandQueues[i].free();
	}

	descriptorPool.shutdown();

	vkDestroyDevice(device, nullptr);
}

void VulkanDevice::createCommandQueues(const QueueFamilyIndices& inQueueFamilyIndices)
{
	commandQueues.resize((uint8_t)EQueueFamily::COUNT);

	const uint32_t graphicsFamily = queueFamilyIndices.graphicsFamily.value();
	VulkanCommandQueue& graphicsCommandQueue = commandQueues[(uint8_t)EQueueFamily::GRAPHICS];
	graphicsCommandQueue.create(device, graphicsFamily);

	const uint32_t presentFamily = queueFamilyIndices.presentFamily.value();
	VulkanCommandQueue& presentCommandQueue = commandQueues[(uint8_t)EQueueFamily::PRESENT];
	presentCommandQueue.create(device, presentFamily);

	const uint32_t transferFamily = queueFamilyIndices.transferFamily.value();
	VulkanCommandQueue& transferCommandQueue = commandQueues[(uint8_t)EQueueFamily::TRANSFER];
	transferCommandQueue.create(device, transferFamily);
}

GPUAllocHandle VulkanDevice::createAndFillBuffer(size_t inSize, void* inData, EBufferType inType)
{
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	GPUAllocHandle stagingBufferHandle = createBuffer(inSize, usage, queueFamilyIndices, propertyFlags);
	if (!stagingBufferHandle.isValid())
	{
		return false;
	}

	VulkanBuffer& stagingBuffer = getBuffer(stagingBufferHandle);
	stagingBuffer.mapAndFill(inData, inSize);

	usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | getVkBufferType(inType);
	propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUAllocHandle vertexBufferHandle = createBuffer(inSize, usage, queueFamilyIndices, propertyFlags);
	if (!vertexBufferHandle.isValid())
	{
		return false;
	}

	const VulkanBuffer& vertexBuffer = getBuffer(vertexBufferHandle);
	VulkanCommandQueue VulkanCommandQueue = getTransferCommandQueue();
	if (!VulkanCommandQueue.isValid())
	{
		VulkanCommandQueue = getGraphicsCommandQueue();
	}

	// note: need to do this cause previous buffer allocation has underlying vector reallocating so previous ref is stale
	stagingBuffer = getBuffer(stagingBufferHandle);
	VulkanCommandQueue.executeCopyBufferCommand(stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), inSize);
	destroyBuffer(stagingBufferHandle);

	return vertexBufferHandle;
}

GPUAllocHandle VulkanDevice::createBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties)
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
	VulkanDevice::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, inProperties, memoryType);
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateMemory failed" << std::endl;
		return GPUAllocHandle::invalid;
	}

	result = vkBindBufferMemory(device, buffer, memory, 0);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkBindBufferMemory failed" << std::endl;
		return GPUAllocHandle::invalid;
	}

	buffers.push_back(VulkanBuffer(device, buffer, memory, allocInfo.allocationSize));
	return GPUAllocHandle((uint32_t)(buffers.size() - 1));
}

VulkanBuffer& VulkanDevice::getBuffer(GPUAllocHandle inHandle)
{
	return buffers.at(inHandle.id);
}

const VulkanBuffer& VulkanDevice::getBuffer(GPUAllocHandle inHandle) const
{
	return buffers.at(inHandle.id);
}

void VulkanDevice::destroyBuffer(GPUAllocHandle handle)
{
	VulkanBuffer& VulkanBuffer = buffers.at(handle.id);
	vkDestroyBuffer(device, VulkanBuffer.getBuffer(), nullptr);
	vkFreeMemory(device, VulkanBuffer.getMemory(), nullptr);
}

void VulkanDevice::resetFrameData(uint32_t inFrameId)
{
	VulkanBuffer& buffer = getBuffer(frameUniformBuffers[inFrameId]);
	buffer.reset();
}

BufferHandle VulkanDevice::pushFrameData(void* inData, uint32_t inSize, VulkanBuffer& inBuffer)
{
	return inBuffer.push_back(inData, inSize, minUniformBufferOffsetAlignment);
}

BufferHandle VulkanDevice::pushMaterialData(void* inData, uint32_t inSize)
{
	// todo: should be determined dynamically, but right now there's only one
	const uint32_t bufferHandle = 0; // is there space left in this buffer ?
	VulkanBuffer& buffer = getBuffer(materialBuffers[bufferHandle]);
	BufferHandle handle = buffer.push_back(inData, inSize, minUniformBufferOffsetAlignment);
	handle.handle = bufferHandle;
	return handle;
}

const uint8_t* VulkanDevice::getMaterialInstanceBuffer(BufferHandle inHandle) const
{
	const uint8_t* memory = (uint8_t*)getBuffer(materialBuffers[inHandle.handle]).getMappedMemory();
	return memory + inHandle.offset;
}

uint32_t VulkanDevice::getMaterialInstanceCompatibleDescriptorSet()
{
	// todo
	return 0;
}

// pipelines
uint32_t VulkanDevice::createPipeline(VkRenderPass inRenderPass
	, VkExtent2D inSwapchainExtent
	, const std::vector<ShaderStageInfo>& inShaderStageInfos
	, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs
	, uint32_t inVertexSize
	, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts
	, const std::vector<VkPushConstantRange>& inPushConstantRanges)
{
	pipelines.push_back({});
	uint32_t handle = (uint32_t)pipelines.size() - 1;
	VulkanPipeline& pipeline = pipelines.at(handle);

	pipeline.initialize(device
		, inRenderPass
		, inSwapchainExtent
		, inShaderStageInfos
		, inVertexInputAttributeDescs
		, inVertexSize
		, inDescriptorSetLayouts
		, inPushConstantRanges);

	return handle;
}

void VulkanDevice::getShaderForMaterial(EMaterialBlendMode inBlendMode, EMaterialType inType, std::vector<ShaderStageInfo>& outShaderStageInfos)
{
	if (inBlendMode == EMaterialBlendMode::OPAQUE && inType == EMaterialType::UNLIT)
	{
		outShaderStageInfos = shader::getUnlitMaterialStageInfos();
	}
	else if (inBlendMode == EMaterialBlendMode::OPAQUE && inType == EMaterialType::LIT)
	{
		outShaderStageInfos = shader::getLitMaterialStageInfos();
	}
}

uint32_t VulkanDevice::getCompatiblePipeline(const MaterialProxy& inMaterialProxy) const
{
	// should be in the renderbackend, can't check pipeline from inside the device
	return 0;
}

bool VulkanDevice::createMaterialData(VkDeviceSize inBufferSize
	, GPUAllocHandle& outBufferHandle
	, VkDescriptorSet& outBufferDescriptorSet)
{
	constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value(), indices.graphicsFamily.value() };
	constexpr VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	outBufferHandle = createBuffer(inBufferSize, usageFlags, queueFamilyIndices, memoryFlags);
	getBuffer(outBufferHandle).map(inBufferSize);

	std::vector<DescriptorSetLayoutElement> layoutElements = VulkanDescriptorPool::getMaterialDescriptorSetLayoutElements();
	if (!VulkanDescriptorPool::createDescriptorSetLayout(device, layoutElements, materialDescriptorSetLayout))
	{
		return false;
	}

	const uint8_t descriptorSetCount = 1;
	DescriptorSetLayoutElementValues layoutValues((uint32_t)layoutElements.size(), descriptorSetCount);
	layoutValues.buffers[0] = { outBufferHandle, sizeof(MaterialData) };

	std::vector<VkDescriptorSet> descriptorSets;
	if (!descriptorPool.createDescriptorSets(*this, { materialDescriptorSetLayout }, layoutElements, layoutValues, descriptorSetCount, descriptorSets))
	{
		return false;
	}

	outBufferDescriptorSet = descriptorSets[0];
	return true;
}

GPUAllocHandle VulkanDevice::createTexture(const uint8_t* inData, size_t inWidth, size_t inHeight, size_t inNrChannels)
{
	VkDeviceSize imageSize = inWidth * inHeight * inNrChannels;

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value() };
	GPUAllocHandle dataBufferHandle = createBuffer(imageSize, usage, queueFamilyIndices, properties);
	if (!dataBufferHandle.isValid())
	{
		return false;
	}

	VulkanBuffer& VulkanBuffer = getBuffer(dataBufferHandle);
	VulkanBuffer.mapAndFill((void*)inData, imageSize);

	// note: format forced at texture level
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	queueFamilyIndices.push_back(indices.graphicsFamily.value());
	GPUAllocHandle imageHandle = createImage((uint32_t)inWidth, (uint32_t)inHeight, format, tiling, usage, queueFamilyIndices, properties);
	if (!imageHandle.isValid())
	{
		return false;
	}

	VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	VulkanImage& VulkanImage = getImage(imageHandle);
	getTransferCommandQueue().executeTransitionImageLayoutCommand(VulkanImage.getImage(), format, oldLayout, newLayout);
	getTransferCommandQueue().executeCopyBufferToImageCommand(VulkanBuffer.getBuffer(), VulkanImage.getImage(), (uint32_t)inWidth, (uint32_t)inHeight);

	oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	getGraphicsCommandQueue().executeTransitionImageLayoutCommand(VulkanImage.getImage(), format, oldLayout, newLayout);
	destroyBuffer(dataBufferHandle);

	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageView view = VulkanImage::createImageView(device, VulkanImage.getImage(), format, aspectFlags);
	VulkanImage.setTextureView(view);

	return imageHandle;
}

GPUAllocHandle VulkanDevice::createImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties)
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
	VulkanDevice::findMemoryType(getPhysicalDevice(), memRequirements.memoryTypeBits, inProperties, memoryType);
	allocInfo.memoryTypeIndex = memoryType;

	result = vkAllocateMemory(getDevice(), &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateMemory failed" << std::endl;
		return false;
	}
	vkBindImageMemory(getDevice(), image, memory, 0);

	images.push_back(VulkanImage(device, image, memory));
	return GPUAllocHandle((uint32_t)(images.size() - 1));
}

VulkanImage& VulkanDevice::getImage(GPUAllocHandle inHandle)
{
	return images.at(inHandle.id);
}

const VulkanImage& VulkanDevice::getImage(GPUAllocHandle inHandle) const
{
	return images.at(inHandle.id);
}

void VulkanDevice::destroyImage(GPUAllocHandle inHandle)
{
	VulkanImage& image = images.at(inHandle.id);

	vkDestroyImage(device, image.getImage(), nullptr);
	vkDestroyImageView(device, image.getView(), nullptr);
	vkFreeMemory(device, image.getMemory(), nullptr);

	image = VulkanImage(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
}

uint32_t VulkanDevice::addTexture(ResourceHandle inTexture)
{
	const Texture* texture = engine::getResources()->getTexture(inTexture);
	ensure(texture);
	
	TextureProxy& textureProxy = render::getRenderer()->getTextureProxy(texture->getProxyHandle());

	const VulkanImage& image = getImage(textureProxy.getHandle());

	VkDescriptorImageInfo imageInfos;
	imageInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos.sampler = VK_NULL_HANDLE;
	imageInfos.imageView = image.getView();

	// todo: texture management
	// > check if there is an available slot, if not create a new descriptorSet
	// > reserve multiple slots in the same descriptorset
	uint32_t textureSlot = getAvailableTextureSlot();
	VkDescriptorSet set = getTextureDescriptorSet(0);

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = set;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = textureSlot;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pImageInfo = &imageInfos;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

	return textureSlot;
}

uint8_t VulkanDevice::getAvailableTextureSlot()
{
	const uint8_t slot = nextTextureAvailableSlot;
	nextTextureAvailableSlot++;
	return slot;
}

bool VulkanDevice::createFrameData(uint8_t inFrameCount
	, VkDeviceSize inBufferSize
	, std::vector<GPUAllocHandle>& outBufferHandles
	, std::vector<VkDescriptorSet>& outBufferDescriptorSets)
{
	constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const QueueFamilyIndices& indices = getQueueFamilyIndices();
	std::vector<uint32_t> queueFamilyIndices = { indices.transferFamily.value(), indices.graphicsFamily.value() };
	constexpr VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (size_t i = 0; i < inFrameCount; ++i)
	{
		GPUAllocHandle bufferHandle = createBuffer(inBufferSize, usageFlags, queueFamilyIndices, memoryFlags);
		outBufferHandles.push_back(bufferHandle);

		VulkanBuffer& buffer = getBuffer(outBufferHandles[i]);
		buffer.map(inBufferSize);
	}
	
	std::vector<DescriptorSetLayoutElement> layoutElements = VulkanDescriptorPool::getFrameDescriptorSetLayoutElements();

	if (!VulkanDescriptorPool::createDescriptorSetLayout(device, layoutElements, frameUniformBufferLayout))
	{
		return false;
	}
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(renderConfig::getFrameCount(), frameUniformBufferLayout);

	DescriptorSetLayoutElementValues layoutValues((uint32_t)layoutElements.size(), inFrameCount);
	layoutValues.buffers[0] = { outBufferHandles[0], sizeof(FrameData) };
	layoutValues.buffers[1] = { outBufferHandles[1], sizeof(FrameData) };

	if (!descriptorPool.createDescriptorSets(*this, descriptorSetLayouts, layoutElements, layoutValues, inFrameCount, frameUniformBufferDescriptorSets))
	{
		return false;
	}

	return true;
}

bool VulkanDevice::createTextureSampler()
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

	VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateSampler failed" << std::endl;
		return false;
	}

	return true;
}

bool VulkanDevice::createTextureData(VkDescriptorSet& outDescriptorSet)
{
	if (!createTextureSampler())
	{
		return false;
	}

	const uint32_t descriptorSetCount = 1;
	std::vector<DescriptorSetLayoutElement> layoutElements = VulkanDescriptorPool::getTexturesDescriptorSetLayoutElements();
	if (!VulkanDescriptorPool::createDescriptorSetLayout(device, layoutElements, textureDescriptorSetLayout))
	{
		return false;
	}

	DescriptorSetLayoutElementValues layoutValues((uint32_t)layoutElements.size(), descriptorSetCount);
	layoutValues.textures[0] = {};
	layoutValues.samplers[1].sampler = sampler;

	std::vector<VkDescriptorSet> descriptorSets;
	if (!descriptorPool.createDescriptorSets(*this, { textureDescriptorSetLayout }, layoutElements, layoutValues, descriptorSetCount, descriptorSets))
	{
		return false;
	}

	outDescriptorSet = descriptorSets[0];
	return true;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface)
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

bool VulkanDevice::findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType)
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

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice)
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

int VulkanDevice::ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface)
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

	VulkanSwapchain::SwapchainSupportDetails swapchainDetails = VulkanSwapchain::querySwapchainSupport(inPhysicalDevice, inSurface);
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

	VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {};
	robustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &robustness2Features;

	vkGetPhysicalDeviceFeatures2(inPhysicalDevice, &deviceFeatures2);
	// note: to enable VK_NULL_HANDLE in shaders, for texture array
	if (robustness2Features.nullDescriptor == VK_FALSE)
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

bool VulkanDevice::pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice)
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

bool VulkanDevice::createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface)
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

	// 1. Check support during physical device selection
	VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {};
	robustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
	robustness2Features.nullDescriptor = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &robustness2Features;
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