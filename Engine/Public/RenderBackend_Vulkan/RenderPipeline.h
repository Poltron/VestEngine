#pragma once

#include <string>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "RenderBackend_Vulkan/RenderStructs.h"

enum class DataFormat : uint8_t
{
	R32G32B32A32,
	R32G32B32,
	R32G32,
	R32,
};

struct VertexInputAttributeDesc
{
	DataFormat format;
	uint32_t offset;
};

enum ShaderStageFlagBits : uint8_t
{
	NONE = 0x00,
	VERTEX = 0x01,
	FRAGMENT = 0x02
};

struct ShaderStageInfo
{
	std::string relativePath;
	ShaderStageFlagBits stage;
};
typedef uint8_t ShaderStageFlags;

struct DescriptorSetLayoutBindingDesc
{
	uint32_t binding;
	DescriptorType type;
	ShaderStageFlags stage;
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

	bool initialize(VkDevice inDevice
		, VkRenderPass inRenderPass
		, VkExtent2D inSwapchainExtent
		, const std::vector<DescriptorSetLayoutBindingDesc>& inDescriptorSetLayoutBindingDescs
		, const std::vector<ShaderStageInfo>& inShaderStageInfos
		, uint32_t inVertexSize
		, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs);
	void destroy();

	bool createDescriptorSetLayout(const std::vector<DescriptorSetLayoutBindingDesc>& inDescriptorSetLayoutBindings, VkDescriptorSetLayout& outDescriptorSetLayout);
	bool createPipelineLayout(VkDescriptorSetLayout inDescriptorSetLayout, VkPipelineLayout& outPipelineLayout);
	bool createPipeline(VkExtent2D inSwapchainExtent, VkRenderPass inRenderPass, const std::vector<ShaderStageInfo>& inShaderStageInfos, uint32_t inVertexSize, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs, VkPipeline& outPipeline);
};