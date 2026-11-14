#include "RenderBackend_Vulkan/VulkanShaderHelper.h"

#include <spirv_reflect.h>

namespace shader
{
	VkShaderStageFlags getShaderStageFlags(ShaderStageFlags inShaderStageFlags)
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

	// todo: add outputs
	// - ShaderParameter ( material data )
	// - PushConstants ( instance data ) 
	// - VertexInputs
	void extractShaderParameters(const std::vector<char>& inBytes, ShaderParameters& outShaderParameters)
	{
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(inBytes.size(), inBytes.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t var_count = 0;
		result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		// contains input ( pos/norm/texcoord)
		std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
		result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (size_t i_var = 0; i_var < input_vars.size(); ++i_var)
		{
			const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
			if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
			{
				continue;
			}

			// todo:
			// - constitute std::vector<VertexInputAttributeDesc>

			//VkVertexInputAttributeDescription attr_desc{};
			//attr_desc.location = refl_var.location;
			//attr_desc.binding = binding_description.binding;
			//attr_desc.format = static_cast<VkFormat>(refl_var.format);
			//attr_desc.offset = 0;  // final offset computed below after sorting.
			//attribute_descriptions.push_back(attr_desc);
		}

		var_count = 0;
		result = spvReflectEnumerateDescriptorBindings(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorBinding*> descriptorBindings(var_count);
		result = spvReflectEnumerateDescriptorBindings(&module, &var_count, descriptorBindings.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		// todo:
		// - extract descriptor sets used
		// - for now, assume pretend cameraBlock / lightBlock are the same on every shader
		// - texture / texture sampler are a combo that are opt-in
		// - material data is the aim for extracting parameters and should fill outShaderParameters
		
		/////
		var_count = 0;
		result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		// push constant blocks, so InstanceData
		std::vector<SpvReflectBlockVariable*> constantBlocks(var_count);
		result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, constantBlocks.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		// todo:
		// - idk ? is instance data changing from shader to shader ? not sure yet
		// - how to handle debug primitive shader ?

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
}