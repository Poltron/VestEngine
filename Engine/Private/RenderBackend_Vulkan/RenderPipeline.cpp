#include "RenderBackend_Vulkan/RenderPipeline.h"

#include <array>
#include <vector>

#include "Core/Ensure.h"
#include "Core/FileSystem.h"

namespace
{
	bool createShaderModule(VkDevice inDevice, const std::vector<char>& inCode, VkShaderModule& outShaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = inCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(inCode.data());

		VkResult result = vkCreateShaderModule(inDevice, &createInfo, nullptr, &outShaderModule);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Error: vkCreateShaderModule failed" << std::endl;
			return false;
		}
		return true;
	}

	VkVertexInputBindingDescription createBindingDescription(uint32_t inBinding, uint32_t inStride)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = inBinding;
		bindingDescription.stride = inStride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkFormat getFormat(DataFormat inFormat)
	{
		switch (inFormat)
		{
			case DataFormat::R32G32B32A32:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			case DataFormat::R32G32B32:
				return VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case DataFormat::R32G32:
				return VK_FORMAT_R32G32_SFLOAT;
				break;
			case DataFormat::R32:
				return VK_FORMAT_R32_SFLOAT;
				break;
		}

		ensure(false);
		return VkFormat::VK_FORMAT_UNDEFINED;
	}

	uint32_t getFormatSize(DataFormat inFormat)
	{
		switch (inFormat)
		{
		case DataFormat::R32G32B32A32:
			return 4;
			break;
		case DataFormat::R32G32B32:
			return 3;
			break;
		case DataFormat::R32G32:
			return 2;
			break;
		case DataFormat::R32:
			return 1;
			break;
		}

		ensure(false);
		return 0;
	}

	void createAttributeDescriptions(const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs, uint32_t inBinding, std::vector<VkVertexInputAttributeDescription>& outVkVertexInputAttributeDescription)
	{
		outVkVertexInputAttributeDescription.clear();
		for (uint32_t i = 0; i < inVertexInputAttributeDescs.size(); ++i)
		{
			VkVertexInputAttributeDescription vkElement{};
			vkElement.binding = inBinding;
			vkElement.location = i;
			vkElement.format = getFormat(inVertexInputAttributeDescs[i].format);
			vkElement.offset = inVertexInputAttributeDescs[i].offset;
			outVkVertexInputAttributeDescription.push_back(vkElement);
		}
	}

	VkShaderStageFlags getShaderStageFlags(ShaderStageFlags inShaderStageFlags)
	{
		VkShaderStageFlags shader = 0;
		if (inShaderStageFlags & ShaderStageFlagBits::FRAGMENT)
		{
			shader = shader | VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		if (inShaderStageFlags & ShaderStageFlagBits::VERTEX)
		{
			shader = shader | VK_SHADER_STAGE_VERTEX_BIT;
		}
		return shader;
	}

	void createVkDescriptorSetLayoutBindings(const std::vector<DescriptorSetLayoutBindingDesc>& inDescriptorSetLayoutBindings, std::vector<VkDescriptorSetLayoutBinding>& outVkDescriptorSetLayoutBindings)
	{
		outVkDescriptorSetLayoutBindings.clear();
		for (const DescriptorSetLayoutBindingDesc& descriptorSetLayoutBinding : inDescriptorSetLayoutBindings)
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

	void createVkPipelineShaderStageCreateInfos(VkDevice inDevice, const std::vector<ShaderStageInfo>& inDescriptorSetLayoutBindings, std::vector<VkPipelineShaderStageCreateInfo>& outVkPipelineShaderStageCreateInfos, std::vector<VkShaderModule>& outShaderModules)
	{
		outVkPipelineShaderStageCreateInfos.clear();

		for (const auto& descriptorSetLayoutBinding : inDescriptorSetLayoutBindings)
		{
			std::vector<char> fileContent;
			File::readFile(File::getProjectDir() + descriptorSetLayoutBinding.relativePath, fileContent);
			VkShaderModule shaderModule;
			createShaderModule(inDevice, fileContent, shaderModule);
			outShaderModules.push_back(shaderModule);

			VkPipelineShaderStageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.stage = (VkShaderStageFlagBits)getShaderStageFlags(descriptorSetLayoutBinding.stage);
			info.module = shaderModule;
			info.pName = "main";
			outVkPipelineShaderStageCreateInfos.push_back(info);
		}
	}
}

bool RenderPipeline::initialize(VkDevice inDevice
	, VkRenderPass inRenderPass
	, VkExtent2D inSwapchainExtent
	, const std::vector<DescriptorSetLayoutBindingDesc>& inDescriptorSetLayoutBindingDescs
	, const std::vector<ShaderStageInfo>& inShaderStageInfos
	, uint32_t inVertexSize
	, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs)
{
	device = inDevice;

	if (!createDescriptorSetLayout(inDescriptorSetLayoutBindingDescs, descriptorSetLayout))
	{
		return false;
	}

	if (!createPipelineLayout(descriptorSetLayout, pipelineLayout))
	{
		return false;
	}

	if (!createPipeline(inSwapchainExtent, inRenderPass, inShaderStageInfos, inVertexSize, inVertexInputAttributeDescs, pipeline))
	{
		return false;
	}

	return true;
}

void RenderPipeline::destroy()
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

bool RenderPipeline::createDescriptorSetLayout(const std::vector<DescriptorSetLayoutBindingDesc>& inDescriptorSetLayoutBindings, VkDescriptorSetLayout& outDescriptorSetLayout)
{
	std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindings;
	createVkDescriptorSetLayoutBindings(inDescriptorSetLayoutBindings, vkDescriptorSetLayoutBindings);

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)vkDescriptorSetLayoutBindings.size();
	layoutInfo.pBindings = vkDescriptorSetLayoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateDescriptorSetLayout failed" << std::endl;
		return false;
	}
	return true;
}

bool RenderPipeline::createPipelineLayout(VkDescriptorSetLayout inDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &inDescriptorSetLayout;

	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreatePipelineLayout failed" << std::endl;
		return false;
	}
	return true;
}

bool RenderPipeline::createPipeline(VkExtent2D inSwapchainExtent, VkRenderPass inRenderPass, const std::vector<ShaderStageInfo>& inShaderStageInfos, uint32_t inVertexSize, const std::vector<VertexInputAttributeDesc>& inVertexInputAttributeDescs, VkPipeline& outPipeline)
{
	uint32_t binding = 0;

	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
	createVkPipelineShaderStageCreateInfos(device, inShaderStageInfos, vkShaderStages, shaderModules);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription bindingDescription = createBindingDescription(binding, inVertexSize);
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	std::vector<VkVertexInputAttributeDescription> vkInputAttributeDescriptions;
	createAttributeDescriptions(inVertexInputAttributeDescs, binding, vkInputAttributeDescriptions);
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vkInputAttributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = vkInputAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)inSwapchainExtent.width;
	viewport.height = (float)inSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = inSwapchainExtent;

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)vkShaderStages.size();
	pipelineInfo.pStages = vkShaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = inRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: vkCreateGraphicsPipeline failed" << std::endl;
		return false;
	}

	//
	for (size_t i = 0; i < shaderModules.size(); ++i)
	{
		vkDestroyShaderModule(device, shaderModules[i], nullptr);
	}

	return true;
}