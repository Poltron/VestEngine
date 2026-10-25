#pragma once

#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/CommandContext.h"
#include "RenderBackend_Vulkan/RenderStructs.h"

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

struct DescriptorPoolElement
{
	DescriptorType type;
	uint32_t count;
};

class RenderDevice
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices queueFamilyIndices;

	std::vector<RenderBuffer> renderBuffers;
	std::vector<RenderImage> renderImages;
	VkDescriptorPool mainDescriptorPool;

	std::vector<CommandContext> commandContexts;

public:
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	VkDevice getDevice() const { return device; }
	const QueueFamilyIndices& getQueueFamilyIndices() const { return queueFamilyIndices; }
	VkDescriptorPool getMainDescriptorPool() const { return mainDescriptorPool; }

	CommandContext& getGraphicsCommandContext() { return commandContexts[(uint8_t)EQueueFamily::GRAPHICS]; }
	CommandContext& getPresentCommandContext() { return commandContexts[(uint8_t)EQueueFamily::PRESENT]; }
	CommandContext& getTransferCommandContext() { return commandContexts[(uint8_t)EQueueFamily::TRANSFER]; }

	bool initialize(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers, uint32_t inFrameCount);
	void shutdown();
	void createCommandContexts(const QueueFamilyIndices& inQueueFamilyIndices);

	GPUHandle createAndFillBuffer(size_t inSize, void* inData, EBufferType inType);
	GPUHandle createBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	const RenderBuffer& getBuffer(GPUHandle inHandle) const;
	bool fillBuffer(GPUHandle inHandle, void* inData, size_t inDataSize);
	void destroyBuffer(GPUHandle inHandle);

	GPUHandle createImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, const std::vector<uint32_t>& inQueueIndices, VkMemoryPropertyFlags inProperties);
	RenderImage& getImage(GPUHandle inHandle);
	const RenderImage& getImage(GPUHandle inHandle) const;
	void destroyImage(GPUHandle inHandle);

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice, VkSurfaceKHR inSurface);
	static bool findMemoryType(VkPhysicalDevice inPhysicalDevice, uint32_t inTypeFilter, VkMemoryPropertyFlags inProperties, uint32_t& outMemoryType);

private:
	bool checkDeviceExtensionSupport(VkPhysicalDevice inPhysicalDevice);
	int ratePhysicalDevice(VkPhysicalDevice inPhysicalDevice, VkSurfaceKHR inSurface);
	bool pickPhysicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, VkPhysicalDevice& outPhysicalDevice);
	bool createLogicalDevice(VkInstance inInstance, VkSurfaceKHR inSurface, bool bInEnableValidationLayers, const std::vector<const char*>& inValidationLayers);
	bool createMainDescriptorPool(const std::vector<DescriptorPoolElement>& inDescriptorPoolElement, uint32_t inMaxSets);
};