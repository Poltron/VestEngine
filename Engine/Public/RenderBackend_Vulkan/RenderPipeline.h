#pragma once

#include <string>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "RenderBackend_Vulkan/ShaderStructs.h"

class RenderDevice;

class RenderPipeline
{
	VkDevice device;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

public:
	VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkPipeline getPipeline() const { return pipeline; }

	bool initialize(RenderDevice& inDevice
		, VkRenderPass inRenderPass
		, VkExtent2D inSwapchainExtent
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs
		, uint32_t inVertexSize
		, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts
		, const std::vector<VkPushConstantRange>& inPushConstantRanges);
	void destroy();

private:
	bool createPipelineLayout(const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts, const std::vector<VkPushConstantRange>& inPushConstantRanges, VkPipelineLayout& outPipelineLayout);
	bool createPipeline(VkExtent2D inSwapchainExtent
		, VkRenderPass inRenderPass
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, uint32_t inVertexSize
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs
		, VkPipeline& outPipeline);
};