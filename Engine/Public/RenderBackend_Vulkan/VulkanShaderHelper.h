#pragma once

#include <spirv_reflect.h>
#include <vulkan/vulkan.h>

#include "Render/ShaderStructs.h"

namespace shader
{
	inline VkShaderStageFlags getShaderStageFlags(ShaderStageFlags inShaderStageFlags)
	{
		VkShaderStageFlags shader = 0;
		if (inShaderStageFlags & EShaderStageFlagBits::FRAGMENT)
		{
			shader = shader | VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		if (inShaderStageFlags & EShaderStageFlagBits::VERTEX)
		{
			shader = shader | VK_SHADER_STAGE_VERTEX_BIT;
		}
		return shader;
	}

	void getShaderParameters(const std::vector<char>& inBytes, ShaderParameters& outShaderParameters)
	{
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(inBytes.size(), inBytes.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t var_count = 0;
		result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
		result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		ShaderParameters shaderParameter;

		for (size_t i_var = 0; i_var < input_vars.size(); ++i_var)
		{
			const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
			if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
			{
				continue;
			}

			//VkVertexInputAttributeDescription attr_desc{};
			//attr_desc.location = refl_var.location;
			//attr_desc.binding = binding_description.binding;
			//attr_desc.format = static_cast<VkFormat>(refl_var.format);
			//attr_desc.offset = 0;  // final offset computed below after sorting.
			//attribute_descriptions.push_back(attr_desc);
		}

		// Output variables, descriptor bindings, descriptor sets, and push constants
		// can be enumerated and extracted using a similar mechanism.

		spvReflectDestroyShaderModule(&module);
	}

	bool createVkShaderModule(VkDevice inDevice, const std::vector<char>& inCode, VkShaderModule& outShaderModule)
	{
		if (inCode.size() == 0)
		{
			outShaderModule = VK_NULL_HANDLE;
			return false;
		}

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

	void createVkPipelineShaderStageCreateInfos(VkDevice inDevice, const ShaderStageInfo& inDescriptorSetLayoutBindings, std::vector<VkPipelineShaderStageCreateInfo>& outVkPipelineShaderStageCreateInfos, std::vector<VkShaderModule>& outShaderModules)
	{
		outVkPipelineShaderStageCreateInfos.clear();

		//for (const auto& descriptorSetLayoutBinding : inDescriptorSetLayoutBindings)
		//{
		//	std::vector<char> fileContent;

		//	File::readFile(File::getProjectDir() + descriptorSetLayoutBinding.relativePath, fileContent);
		//	VkShaderModule shaderModule;
		//	createShaderModule(inDevice, fileContent, shaderModule);
		//	outShaderModules.push_back(shaderModule);

		//	VkPipelineShaderStageCreateInfo info{};
		//	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//	info.stage = (VkShaderStageFlagBits)getShaderStageFlags(descriptorSetLayoutBinding.stage);
		//	info.module = shaderModule;
		//	info.pName = "main";
		//	outVkPipelineShaderStageCreateInfos.push_back(info);
		//}
	}
}