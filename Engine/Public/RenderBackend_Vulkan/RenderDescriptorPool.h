#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/RenderStructs.h"

class RenderDevice;

enum class EDescriptorType : uint8_t
{
	UNIFORM_BUFFER,
	UNIFORM_BUFFER_DYNAMIC,
	TEXTURE_SAMPLER
};

inline VkDescriptorType getDescriptorType(EDescriptorType inDescriptorType)
{
	switch (inDescriptorType)
	{
	case EDescriptorType::UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case EDescriptorType::UNIFORM_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	case EDescriptorType::TEXTURE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	}

	ensure(false);
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

struct DescriptorPoolElement
{
	EDescriptorType type;
	uint32_t count;
};

struct DescriptorSetLayoutElement
{
	uint32_t binding;
	EDescriptorType type;
	ShaderStageFlags stage;
	uint32_t dstArrayElement;
};

struct DescriptorSetLayoutElementBufferValue
{
	GPUHandle bufferHandle;
};

struct DescriptorSetLayoutElementImageValue
{
	GPUHandle imageHandle;
	VkSampler sampler;
};

struct DescriptorSetLayoutElementValues
{
	std::vector<DescriptorSetLayoutElementBufferValue> buffers;
	std::vector<DescriptorSetLayoutElementImageValue> images;

	DescriptorSetLayoutElementValues(uint32_t inElementCount, uint32_t inDescriptorCount)
	{
		buffers.resize(inElementCount * inDescriptorCount);
		images.resize(inElementCount * inDescriptorCount);
	}
};

class RenderDescriptorPool
{
	VkDevice device;
	VkDescriptorPool pool;

public:
	VkDescriptorPool getPool() { return pool; }

	bool initialize(const RenderDevice& inDevice, const std::vector<DescriptorPoolElement>& inPoolElements, uint32_t inMaxSets);
	void shutdown();

	bool createDescriptorSets(RenderDevice& inDevice
		, const std::vector<VkDescriptorSetLayout>& inLayouts
		, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
		, const DescriptorSetLayoutElementValues& inLayoutValues
		, uint32_t inDescriptorSetCount
		, std::vector<VkDescriptorSet>& outDescriptorSets);
	void releaseDescriptorSet(uint32_t inCount, const VkDescriptorSet* inSets);

	static bool createDescriptorSetLayout(VkDevice inDevice
		, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
		, VkDescriptorSetLayout& outDescriptorSetLayout);
};