#pragma once

#include <optional>
#include <set>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "Core/Allocators/StackAllocator.h"
#include "Core/ResourceHandle.h"
#include "Core/Resources/Material.h"
#include "Core/Resources/MaterialInstance.h"
#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/CommandContext.h"
#include "RenderBackend_Vulkan/RenderDescriptorPool.h"
#include "RenderBackend_Vulkan/RenderConfig.h"

class RenderImage;
class RenderBuffer;

enum class EQueueFamily : uint8_t
{
	GRAPHICS = 0,
	PRESENT,
	TRANSFER,
	COUNT
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() const
	{
		return graphicsFamily.has_value()
			&& presentFamily.has_value()
			&& transferFamily.has_value();
	}

	std::set<uint32_t> getIndicesSet() const
	{ 
		return std::set<uint32_t>({ graphicsFamily.value(), presentFamily.value(), transferFamily.value() });
	}
};

class RenderDevice
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices queueFamilyIndices;

	std::vector<CommandContext> commandContexts;

	std::vector<RenderBuffer> renderBuffers;
	std::vector<RenderImage> images;
	VkSampler sampler;
	
	RenderDescriptorPool descriptorPool;

	uint32_t minUniformBufferOffsetAlignment;
	uint32_t maxUniformBufferRange;

	std::vector<GPUHandle> frameUniformBuffers;
	std::vector<VkDescriptorSet> frameUniformBufferDescriptorSets;
	VkDescriptorSetLayout frameUniformBufferLayout;

	VkDescriptorSet textureDescriptorSet;
	VkDescriptorSetLayout textureDescriptorSetLayout;

	GPUHandle materialBuffer = GPUHandle::invalid;
	VkDescriptorSet materialDescriptorSet;
	VkDescriptorSetLayout materialDescriptorSetLayout;

	uint8_t nextTextureAvailableSlot;

public:
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	VkDevice getDevice() const { return device; }
	const QueueFamilyIndices& getQueueFamilyIndices() const { return queueFamilyIndices; }
	RenderDescriptorPool getDescriptorPool() const { return descriptorPool; }

	CommandContext& getGraphicsCommandContext() { return commandContexts[(uint8_t)EQueueFamily::GRAPHICS]; }
	CommandContext& getPresentCommandContext() { return commandContexts[(uint8_t)EQueueFamily::PRESENT]; }
	CommandContext& getTransferCommandContext() { return commandContexts[(uint8_t)EQueueFamily::TRANSFER]; }

	GPUHandle getFrameBufferHandle(uint32_t inFrameIndex) const { return frameUniformBuffers[inFrameIndex]; }
	VkDescriptorSet getFrameBufferDescriptorSet(uint32_t inFrameIndex) const { return frameUniformBufferDescriptorSets[inFrameIndex]; }
	VkDescriptorSetLayout getFrameBufferDescriptorSetLayout() const { return frameUniformBufferLayout; }

	VkDescriptorSet getTextureDescriptorSet() const { return textureDescriptorSet; }
	VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return textureDescriptorSetLayout; }

	GPUHandle getMaterialBufferHandle() const { return materialBuffer; }
	VkDescriptorSet getMaterialBufferDescriptorSet() const { return materialDescriptorSet; }
	VkDescriptorSetLayout getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout; }

public:
	bool initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
	void shutdown();
	void createCommandContexts(const QueueFamilyIndices& inQueueFamilyIndices);

	// resources
	//
	GPUHandle createAndFillBuffer(size_t inSize, void* inData, EBufferType inType);
	GPUHandle createBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	RenderBuffer& getBuffer(GPUHandle inHandle);
	const RenderBuffer& getBuffer(GPUHandle inHandle) const;
	void destroyBuffer(GPUHandle inHandle);

	//
	GPUHandle createImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	RenderImage& getImage(GPUHandle inHandle);
	const RenderImage& getImage(GPUHandle inHandle) const;
	void destroyImage(GPUHandle inHandle);

	// rendering data, descriptor sets
	//
	void resetFrameData(uint32_t inFrameId);
	BufferHandle pushFrameData(void* inData, uint32_t inSize, uint32_t inFrameId);

	//
	uint32_t addTexture(ResourceHandle inTexture);
	uint8_t getAvailableTextureSlot();

	//
	BufferHandle pushMaterialData(void* inData, uint32_t inSize);
	const uint8_t* getMaterialInstanceBuffer(uint32_t inOffset) const;
	uint32_t getMaterialInstanceCompatibleDescriptorSet();

	//DescriptorSetHandle createMaterialDescriptorSet(GPUHandle inBuffer, size_t inBufferSize);
	//void destroyMaterialDescriptorSets(DescriptorSetHandle inHandle);
	VkDescriptorSet getMaterialDescriptorSet(uint32_t inHandle) const;

	// should be in renderbackend
	void getShaderForMaterial(EMaterialBlendMode inBlendMode, EMaterialType inType, std::vector<ShaderStageInfo>& outShaderStageInfos);
	bool checkCompatiblePipeline(const std::vector<ShaderStageInfo>& inShaderStageInfos) const;

private:
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface);
	static bool findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType);

	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);
	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice);
	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);

	bool createFrameData(uint8_t inFrameCount
		, VkDeviceSize inBufferSize
		, std::vector<GPUHandle>& outBufferHandles
		, std::vector<VkDescriptorSet>& outBufferDescriptorSets);

	bool createTextureSampler();
	bool createTextureData(uint8_t inTexturesCount
		, VkDescriptorSet& outDescriptorSet);

	bool createMaterialData(VkDeviceSize inBufferSize
		, GPUHandle& outBufferHandle
		, VkDescriptorSet& outBufferDescriptorSet);
};