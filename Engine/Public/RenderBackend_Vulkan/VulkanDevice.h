#pragma once

#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

#include "Core/ResourceHandle.h"
#include "Core/Resources/Material.h"
#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/VulkanDescriptorPool.h"
#include "Render/Resources/MaterialProxy.h"

class VulkanBuffer;
class VulkanCommandQueue;
class VulkanImage;
class VulkanInstance;
class VulkanPipeline;

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

class VulkanDevice
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;

	QueueFamilyIndices queueFamilyIndices;
	std::vector<VulkanCommandQueue> commandQueues;

	std::vector<VulkanBuffer> buffers;
	std::vector<VulkanImage> images;
	VkSampler sampler;
	
	VulkanDescriptorPool descriptorPool;

	uint32_t minUniformBufferOffsetAlignment;
	uint32_t maxUniformBufferRange;

	std::vector<GPUAllocHandle> frameUniformBuffers;
	std::vector<VkDescriptorSet> frameUniformBufferDescriptorSets;
	VkDescriptorSetLayout frameUniformBufferLayout;

	std::vector<VkDescriptorSet> textureDescriptorSets;
	VkDescriptorSetLayout textureDescriptorSetLayout;

	std::vector<GPUAllocHandle> materialBuffers;
	std::vector<VkDescriptorSet> materialDescriptorSets;
	VkDescriptorSetLayout materialDescriptorSetLayout;

	uint8_t nextTextureAvailableSlot;

	std::vector<VulkanPipeline> pipelines;

public:
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	VkDevice getDevice() const { return device; }
	const QueueFamilyIndices& getQueueFamilyIndices() const { return queueFamilyIndices; }
	VulkanDescriptorPool getDescriptorPool() const { return descriptorPool; }

	VulkanCommandQueue& getGraphicsCommandQueue();
	VulkanCommandQueue& getPresentVulkanCommandQueue();
	VulkanCommandQueue& getTransferCommandQueue();

	GPUAllocHandle getFrameBufferHandle(uint32_t inFrameIndex) const { return frameUniformBuffers[inFrameIndex]; }
	VkDescriptorSet getFrameBufferDescriptorSet(uint32_t inFrameIndex) const { return frameUniformBufferDescriptorSets[inFrameIndex]; }
	VkDescriptorSetLayout getFrameBufferDescriptorSetLayout() const { return frameUniformBufferLayout; }

	VkDescriptorSet getTextureDescriptorSet(uint32_t inHandle) const { return textureDescriptorSets[inHandle]; }
	VkDescriptorSetLayout getTextureDescriptorSetLayout() const { return textureDescriptorSetLayout; }

	GPUAllocHandle getMaterialBufferHandle(uint32_t inHandle) const { return materialBuffers[inHandle]; }
	VkDescriptorSet getMaterialBufferDescriptorSet(uint32_t inHandle) const { return materialDescriptorSets[inHandle]; }
	VkDescriptorSetLayout getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout; }

	const VulkanPipeline& getPipeline(uint32_t inHandle) const;

public:
	bool initialize(const VulkanInstance& inInstance, VkSurfaceKHR inSurface);
	void shutdown();

	// resources
	//
	GPUAllocHandle createAndFillBuffer(size_t inSize, void* inData, EBufferType inType);
	GPUAllocHandle createBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	VulkanBuffer& getBuffer(GPUAllocHandle inHandle);
	const VulkanBuffer& getBuffer(GPUAllocHandle inHandle) const;
	void destroyBuffer(GPUAllocHandle inHandle);

	//
	GPUAllocHandle createTexture(const uint8_t* inData, size_t inWidth, size_t inHeight, size_t inNrChannels);
	GPUAllocHandle createImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	VulkanImage& getImage(GPUAllocHandle inHandle);
	const VulkanImage& getImage(GPUAllocHandle inHandle) const;
	void destroyImage(GPUAllocHandle inHandle);

	// rendering data, descriptor sets
	//
	void resetFrameData(uint32_t inFrameId);
	BufferHandle pushFrameData(void* inData, uint32_t inSize, VulkanBuffer& inBuffer);

	//
	uint32_t addTexture(ResourceHandle inTexture);
	uint8_t getAvailableTextureSlot();

	//
	BufferHandle pushMaterialData(void* inData, uint32_t inSize);
	const uint8_t* getMaterialInstanceBuffer(BufferHandle inHandle) const;
	uint32_t getMaterialInstanceCompatibleDescriptorSet();

	// pipelines
	uint32_t createPipeline(VkRenderPass inRenderPass
		, VkExtent2D inSwapchainExtent
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs
		, uint32_t inVertexSize
		, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts
		, const std::vector<VkPushConstantRange>& inPushConstantRanges);

	// should be in renderbackend
	void getShaderForMaterial(EMaterialBlendMode inBlendMode, EMaterialType inType, std::vector<ShaderStageInfo>& outShaderStageInfos);
	uint32_t getCompatiblePipeline(const MaterialProxy& inMaterialProxy) const;

private:
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface);
	static bool findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType);

	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);
	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice);
	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface);
	void createCommandQueues(const QueueFamilyIndices& inQueueFamilyIndices);

	bool createFrameData(uint8_t inFrameCount
		, VkDeviceSize inBufferSize
		, std::vector<GPUAllocHandle>& outBufferHandles
		, std::vector<VkDescriptorSet>& outBufferDescriptorSets);

	bool createTextureSampler();
	bool createTextureData(VkDescriptorSet& outDescriptorSet);

	bool createMaterialData(VkDeviceSize inBufferSize
		, GPUAllocHandle& outBufferHandle
		, VkDescriptorSet& outBufferDescriptorSet);
};