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
			binding.stageFlags = shader::getShaderStageFlags(descriptorSetLayoutBinding.stage);
			binding.pImmutableSamplers = nullptr;

			outVkDescriptorSetLayoutBindings.push_back(binding);
		}
	}

	bool createDescriptorSetLayout(VkDevice inDevice
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

	void createWriteDescriptorBuffer(const VulkanBuffer& inBuffer, const VkDeviceSize inDataSize, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, std::vector<VkDescriptorBufferInfo>& outBufferInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outBufferInfos.push_back({ inBuffer.getBuffer(), 0, inDataSize });

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pBufferInfo = &outBufferInfos.back();
	}

	void createWriteDescriptorSampler(const VkSampler& inSampler, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, std::vector<VkDescriptorImageInfo>& outImageInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outImageInfos.push_back({ inSampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pImageInfo = &outImageInfos.back();
	}

	void createWriteDescriptorTextures(VkSampler inSampler, VkImageView inImageView, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, uint8_t inDescriptorCount, std::vector<VkDescriptorImageInfo>& outImageInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		uint32_t startIndex = (uint32_t)outImageInfos.size();
		for (uint32_t i = startIndex; i < startIndex + inDescriptorCount; ++i)
		{
			outImageInfos.push_back({ inSampler, inImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
		}

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		outWriteDescriptorSet.descriptorCount = inDescriptorCount;
		outWriteDescriptorSet.pImageInfo = &outImageInfos[startIndex];
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
	, uint8_t inCount
	, const std::vector<DescriptorSetLayoutElement>& inLayoutElements
	, VkDescriptorSetLayout& outLayout
	, std::vector<VkDescriptorSet>& outDescriptorSets)
{
	if (!createDescriptorSetLayout(device, inLayoutElements, outLayout))
	{
		return false;
	}

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(inCount, outLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = inCount;
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	outDescriptorSets.resize(inCount);
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets.data());
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkAllocateDescriptorSets failed" << std::endl;
		return false;
	}

	// todo: replace with a dumb linear allocator since we just need to pile them up and access
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;
	std::vector<VkDescriptorImageInfo> samplerInfos;
	for (uint32_t descriptorSetIndex = 0; descriptorSetIndex < inCount; descriptorSetIndex++)
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
					const VulkanBuffer& buffer = inDevice.getBuffer(GPUAllocHandle((uint32_t)layoutElement.data1));
					const VkDeviceSize deviceSize = (VkDeviceSize)layoutElement.data2;
					createWriteDescriptorBuffer(buffer
						, deviceSize
						, outDescriptorSets[descriptorSetIndex]
						, layoutElement.binding
						, layoutElement.dstArrayElement
						, bufferInfos
						, writeDescriptors[writeIndex]);
					
					break;
				}
				case EDescriptorType::SAMPLER:
				{
					VkSampler sampler = (VkSampler)layoutElement.data1;
					createWriteDescriptorSampler(sampler
						, outDescriptorSets[descriptorSetIndex]
						, layoutElement.binding
						, layoutElement.dstArrayElement
						, samplerInfos
						, writeDescriptors[writeIndex]);
					break;
				}
				case EDescriptorType::TEXTURE:
				{
					ensure(layoutElement.descriptorCount > 0);

					VkSampler sampler = VK_NULL_HANDLE;
					VkImageView imageView = VK_NULL_HANDLE;
					createWriteDescriptorTextures(sampler
						, imageView
						, outDescriptorSets[descriptorSetIndex]
						, layoutElement.binding
						, layoutElement.dstArrayElement
						, layoutElement.descriptorCount
						, imageInfos
						, writeDescriptors[writeIndex]);
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


std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getFrameDescriptorSetLayoutElements(const std::vector<GPUAllocHandle>& inBuffer, uint32_t inDataSize)
{
	std::vector<DescriptorSetLayoutElement> layoutElements;
	for (size_t i = 0; i < inBuffer.size(); ++i)
	{
		DescriptorSetLayoutElement element;
		element.binding = 0;
		element.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
		element.stage = EShaderStageFlagBits::VERTEX;
		element.dstArrayElement = 0;
		element.descriptorCount = 1;
		element.data1 = (uint64_t)inBuffer[i].id;
		element.data2 = (uint64_t)inDataSize;
		layoutElements.push_back(element);
	}

	return layoutElements;
}

std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getTexturesDescriptorSetLayoutElements(uint32_t inTextureCount, VkSampler inSampler)
{
	DescriptorSetLayoutElement element0{};
	element0.binding = 0;
	element0.type = EDescriptorType::TEXTURE;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	element0.descriptorCount = inTextureCount;
	element0.data1 = (uint64_t)VK_NULL_HANDLE;
	element0.data2 = (uint64_t)VK_NULL_HANDLE;

	DescriptorSetLayoutElement element1{};
	element1.binding = 1;
	element1.type = EDescriptorType::SAMPLER;
	element1.stage = EShaderStageFlagBits::FRAGMENT;
	element1.dstArrayElement = 0;
	element1.descriptorCount = 1;
	element1.data1 = (uint64_t)inSampler;
	element1.data2 = (uint64_t)VK_NULL_HANDLE;

	return { element0, element1 };
}
std::vector<DescriptorSetLayoutElement> VulkanDescriptorPool::getMaterialDescriptorSetLayoutElements(uint32_t inDataSize)
{
	DescriptorSetLayoutElement element0;
	element0.binding = 0;
	element0.type = EDescriptorType::UNIFORM_BUFFER_DYNAMIC;
	element0.stage = EShaderStageFlagBits::FRAGMENT;
	element0.dstArrayElement = 0;
	element0.descriptorCount = 1;
	element0.data1 = (uint64_t)inDataSize;

	return { element0 };
}