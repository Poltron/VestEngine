#pragma once

#include <array>
#include <vector>
#include <vulkan/vulkan.h>

#include "Render/GPUHandle.h"
#include "RenderBackend_Vulkan/VulkanShaderHelper.h"
#include "RenderBackend_Vulkan/VulkanConfig.h"

class VulkanDevice;

enum class EDescriptorType : uint8_t
{
	UNIFORM_BUFFER,
	UNIFORM_BUFFER_DYNAMIC,
	SAMPLER,
	TEXTURE
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
	case EDescriptorType::SAMPLER:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
		break;
	case EDescriptorType::TEXTURE:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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
	uint32_t descriptorCount;
	uint32_t dstArrayElement;
};

struct DescriptorSetLayoutElementBufferValue
{
	GPUHandle bufferHandle = GPUHandle::invalid;
	VkDeviceSize dataSize;
};

struct DescriptorSetLayoutElementTextureValue
{
	std::array<GPUHandle, renderConfig::getTextureArraySize()> imageHandles;

	DescriptorSetLayoutElementTextureValue()
		: imageHandles({ GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid
			, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid
			, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid
			, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid, GPUHandle::invalid })
	{}
};

struct DescriptorSetLayoutElementSamplerValue
{
	VkSampler sampler;
};

struct DescriptorSetLayoutElementValues
{
	std::vector<DescriptorSetLayoutElementBufferValue> buffers;
	std::vector<DescriptorSetLayoutElementTextureValue> textures;
	std::vector<DescriptorSetLayoutElementSamplerValue> samplers;

	DescriptorSetLayoutElementValues(uint32_t inElementCount, uint32_t inDescriptorCount)
	{
		buffers.resize(inElementCount * inDescriptorCount);
		textures.resize(inElementCount * inDescriptorCount);
		samplers.resize(inElementCount * inDescriptorCount);
	}
};

class VulkanDescriptorPool
{
	VkDevice device;
	VkDescriptorPool pool;

public:
	VkDescriptorPool getPool() { return pool; }

	bool initialize(const VulkanDevice& inDevice, const std::vector<DescriptorPoolElement>& inPoolElements, uint32_t inMaxSets);
	void shutdown();

	bool createDescriptorSets(VulkanDevice& inDevice
		, const std::vector<VkDescriptorSetLayout>& inLayouts
		, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
		, const DescriptorSetLayoutElementValues& inLayoutValues
		, uint32_t inDescriptorSetCount
		, std::vector<VkDescriptorSet>& outDescriptorSets);
	void releaseDescriptorSet(uint32_t inCount, const VkDescriptorSet* inSets);

	static bool createDescriptorSetLayout(VkDevice inDevice
		, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
		, VkDescriptorSetLayout& outDescriptorSetLayout);

	static std::vector<DescriptorSetLayoutElement> getFrameDescriptorSetLayoutElements();
	static std::vector<DescriptorSetLayoutElement> getTexturesDescriptorSetLayoutElements();
	static std::vector<DescriptorSetLayoutElement> getMaterialDescriptorSetLayoutElements();
};