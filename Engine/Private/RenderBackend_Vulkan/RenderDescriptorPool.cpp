#include "RenderBackend_Vulkan/RenderDescriptorPool.h"

#include "RenderBackend_Vulkan/RenderBuffer.h"
#include "RenderBackend_Vulkan/RenderDevice.h"
#include "RenderBackend_Vulkan/RenderImage.h"

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
			binding.descriptorCount = 1;
			binding.descriptorType = getDescriptorType(descriptorSetLayoutBinding.type);
			binding.stageFlags = getShaderStageFlags(descriptorSetLayoutBinding.stage);
			binding.pImmutableSamplers = nullptr;

			outVkDescriptorSetLayoutBindings.push_back(binding);
		}
	}

	void createWriteDescriptorBuffer(const RenderBuffer& inBuffer, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, VkDescriptorBufferInfo& outBufferInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outBufferInfos.buffer = inBuffer.getBuffer();
		outBufferInfos.offset = 0;
		outBufferInfos.range = inBuffer.getSize();

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pBufferInfo = &outBufferInfos;
	}

	void createWriteDescriptorTexture(const VkImageView& inImageView, const VkSampler& inSampler, const VkDescriptorSet& inDescriptorSet, uint32_t inBinding, uint32_t inDstArrayElement, VkDescriptorImageInfo& outImageInfos, VkWriteDescriptorSet& outWriteDescriptorSet)
	{
		outImageInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		outImageInfos.imageView = inImageView;
		outImageInfos.sampler = inSampler;

		outWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outWriteDescriptorSet.dstSet = inDescriptorSet;
		outWriteDescriptorSet.dstBinding = inBinding;
		outWriteDescriptorSet.dstArrayElement = inDstArrayElement;
		outWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		outWriteDescriptorSet.descriptorCount = 1;
		outWriteDescriptorSet.pImageInfo = &outImageInfos;
	}
}

bool RenderDescriptorPool::initialize(const RenderDevice& inDevice, const std::vector<DescriptorPoolElement>& inPoolElements, uint32_t inMaxSets)
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

void RenderDescriptorPool::shutdown()
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

bool RenderDescriptorPool::createDescriptorSets(RenderDevice& inDevice
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

					const RenderBuffer& buffer = inDevice.getBuffer(inLayoutValues.buffers[writeIndex].bufferHandle);
					createWriteDescriptorBuffer(buffer, outDescriptorSets[descriptorSetIndex], layoutElement.binding, layoutElement.dstArrayElement, bufferInfos.back(), writeDescriptors[writeIndex]);
					break;
				}
				case EDescriptorType::TEXTURE_SAMPLER:
				{
					imageInfos.push_back({});

					const auto& imageValue = inLayoutValues.images[writeIndex];
					const RenderImage& image = inDevice.getImage(imageValue.imageHandle);
					createWriteDescriptorTexture(image.getView(), imageValue.sampler, outDescriptorSets[descriptorSetIndex], layoutElement.binding, layoutElement.dstArrayElement, imageInfos.back(), writeDescriptors[writeIndex]);
					break;
				}
			}
		}

		vkUpdateDescriptorSets(inDevice.getDevice(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}

	return true;
}

void RenderDescriptorPool::releaseDescriptorSet(uint32_t inCount, const VkDescriptorSet* inSets)
{
	vkFreeDescriptorSets(device, pool, inCount, inSets);
}

bool RenderDescriptorPool::createDescriptorSetLayout(VkDevice inDevice
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