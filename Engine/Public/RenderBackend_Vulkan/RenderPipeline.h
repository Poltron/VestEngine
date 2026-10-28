#pragma once

#include <string>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "RenderBackend_Vulkan/RenderStructs.h"

class RenderDevice;

enum class EDataFormat : uint8_t
{
	R32G32B32A32,
	R32G32B32,
	R32G32,
	R32,
};

struct VertexInputAttributeDesc
{
	EDataFormat format;
	uint32_t offset;
};

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
		, const std::vector<VkPushConstantRange>& inPushConstantRanges
		, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, uint32_t inVertexSize
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs);
	void destroy();

private:
	bool createPipelineLayout(const std::vector<VkPushConstantRange>& inPushConstantRanges, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts, VkPipelineLayout& outPipelineLayout);
	bool createPipeline(VkExtent2D inSwapchainExtent
		, VkRenderPass inRenderPass
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, uint32_t inVertexSize
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs
		, VkPipeline& outPipeline);
};