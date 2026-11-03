#include "RenderBackend_Vulkan/VulkanDescriptorPool.h"

#include "RenderBackend_Vulkan/VulkanBuffer.h"
#include "RenderBackend_Vulkan/VulkanDevice.h"
#include "RenderBackend_Vulkan/VulkanImage.h"

namespace
{
	void createDescriptorSetLayoutBindings(const std::vector<DescriptorSetLayoutElement>& inLayoutElements
		, std::vector<VkDescriptorSetLayoutBinding>& outVkDescriptorSetLayoutBindings)
	{
		outVkDescriptorSetLayoutBindings.clear();

		for (const DescriptorSetLayoutElement& descriptorSetLayoutBinding : inLayoutElements)
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = descriptorSetLayoutBinding.binding;
			binding.descriptorCount = descriptorSetLayoutBinding.descriptorCount;
			binding.descriptorType = getDescriptorType(descriptorSetLayoutBinding.type);
			binding.stageFlags = getShaderStageFlags(descriptorSetLayoutBinding.stage);
			binding.pImmutableSamplers = nullptr;

			outVkDescriptorSetLayoutBindings.push_back(binding);
		}
	}

	void createWriteDescriptorBuffer(const VulkanBuffer& inBuffer, const VkDeviceSize inDataSize, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, VkDescriptorBufferInfo& outBufferInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outBufferInfos.buffer = inBuffer.getBuffer();
		outBufferInfos.offset = 0;
		outBufferInfos.range = inDataSize;

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pBufferInfo = &outBufferInfos;
	}

	void createWriteDescriptorSampler(const VkSampler& inSampler, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, VkDescriptorImageInfo& outImageInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outImageInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		outImageInfos.sampler = inSampler;
		outImageInfos.imageView = VK_NULL_HANDLE;

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pImageInfo = &outImageInfos;
	}

	void createWriteDescriptorTexture(const VkImageView& inImageView, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, uint8_t inDescriptorCount, VkDescriptorImageInfo& outImageInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		outWriteDescriptorSet.descriptorCount = inDescriptorCount;
		outWriteDescriptorSet.pImageInfo = &outImageInfos;
	}
}

bool VulkanDescriptorPool::initialize(const VulkanDevice& inDevice, const std::vector<DescriptorPoolElement>& inPoolElements, uint32_t inMaxSets)
{
	device = inDevice.getDevice();

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (size_t i = 0; i < inPoolElements.size(); ++i)
	{
		const auto& poolElement = inPoolElements[i];

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
	VkResult result = vkCreateDescriptorPool(inDevice.getDevice(), &poolInfo, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDescriptorPool failed" << std::endl;
		return false;
	}
	return true;
}

void VulkanDescriptorPool::shutdown()
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

bool VulkanDescriptorPool::createDescriptorSets(VulkanDevice& inDevice
	, const std::vector<VkDescriptorSetLayout>& inLayouts
	, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
	, const DescriptorSetLayoutElementValues& inLayoutValues
	, uint32_t inDescriptorSetCount
	, std::vector<VkDescriptorSet>& outDescriptorSets)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = inDescriptorSetCount;
	allocInfo.pSetLayouts = inLayouts.data();

	outDescriptorSets.resize(inDescriptorSetCount);
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets.data());
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateDescriptorSets failed" << std::endl;
		return false;
	}

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;
	std::vector<VkDescriptorImageInfo> samplerInfos;
	for (uint32_t descriptorSetIndex = 0; descriptorSetIndex < inDescriptorSetCount; descriptorSetIndex++)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptors(inLayoutElements.size());
		
		for (uint32_t writeIndex = 0; writeIndex < inLayoutElements.size(); ++writeIndex)
		{
			const DescriptorSetLayoutElement& layoutElement = inLayoutElements[writeIndex];

			switch (layoutElement.type)
			{
				case EDescriptorType::UNIFORM_BUFFER:
				case EDescriptorType::UNIFORM_BUFFER_DYNAMIC:
				{
					bufferInfos.push_back({});

					const VulkanBuffer& buffer = inDevice.getBuffer(inLayoutValues.buffers[writeIndex].bufferHandle);
					createWriteDescriptorBuffer(buffer, inLayoutValues.buffers[writeIndex].dataSize, outDescriptorSets[descriptorSetIndex], layoutElement.binding, layoutElement.dstArrayElement, bufferInfos.back(), writeDescriptors[writeIndex]);
					break;
				}
				case EDescriptorType::SAMPLER:
				{
					samplerInfos.push_back({});

					const auto& samplerValue = inLayoutValues.samplers[writeIndex];
					createWriteDescriptorSampler(samplerValue.sampler, outDescriptorSets[descriptorSetIndex], layoutElement.binding, layoutElement.dstArrayElement, samplerInfos.back(), writeDescriptors[writeIndex]);
					break;
				}
				case EDescriptorType::TEXTURE:
				{
					//const auto& textureValue = inLayoutValues.textures[writeIndex];
					ensure(layoutElement.descriptorCount > 0);

					uint32_t startIndex = (uint32_t)imageInfos.size();
					for (uint32_t i = startIndex; i < startIndex + layoutElement.descriptorCount; ++i)
					{
						VkDescriptorImageInfo imageInfo;
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.sampler = VK_NULL_HANDLE;
						imageInfo.imageView = VK_NULL_HANDLE;
						imageInfos.push_back(imageInfo);
					}
					
					createWriteDescriptorTexture(VK_NULL_HANDLE, outDescriptorSets[descriptorSetIndex], layoutElement.binding, layoutElement.dstArrayElement, layoutElement.descriptorCount, imageInfos[startIndex], writeDescriptors[writeIndex]);
					break;
				}
			}
		}

		vkUpdateDescriptorSets(inDevice.getDevice(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}

	return true;
}

void VulkanDescriptorPool::releaseDescriptorSet(uint32_t inCount, const VkDescriptorSet* inSets)
{
	vkFreeDescriptorSets(device, pool, inCount, inSets);
}

bool VulkanDescriptorPool::createDescriptorSetLayout(VkDevice inDevice
	, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
	, VkDescriptorSetLayout& outDescriptorSetLayout)
{
	std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindings;
	createDescriptorSetLayoutBindings(inLayoutElements, vkDescriptorSetLayoutBindings);

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)vkDescriptorSetLayoutBindings.size();
	layoutInfo.pBindings = vkDescriptorSetLayoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(inDevice, &layoutInfo, nullptr, &outDescriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDescriptorSetLayout failed" << std::endl;
		return false;
	}
	return true;
}


std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getFrameDescriptorSetLayoutElements()
{
	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element0.stage = EShaderStageFlagBits::VERTEX;
	element0.dstArrayElement = 0;
	element0.descriptorCount = 1;

	return { element0 };
}

std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getTexturesDescriptorSetLayoutElements()
{
	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::TEXTURE;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	element0.descriptorCount = renderConfig::getTextureArraySize();

	DescriptorSetLayoutElement element1;
	element1.binding = 1;
	element1.type = EDescriptorType::SAMPLER;
	element1.stage = EShaderStageFlagBits::FRAGMENT;
	element1.dstArrayElement = 0;
	element1.descriptorCount = 1;

	return { element0, element1 };
}
std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getMaterialDescriptorSetLayoutElements()
{
	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	element0.descriptorCount = 1;

	return { element0 };
}