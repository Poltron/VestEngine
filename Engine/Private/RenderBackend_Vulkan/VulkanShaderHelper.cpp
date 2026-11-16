#include "RenderBackend_Vulkan/VulkanShaderHelper.h"

#include <spirv_reflect.h>

#include "RenderBackend_Vulkan/VulkanRenderBackend.inl"

namespace
{
	void extractVertexInputs(SpvReflectShaderModule& module, std::vector<VertexInputAttribute>& outVertexInputs, uint32_t& outVertexInputsSize)
	{
		uint32_t count;
		SpvReflectResult result = spvReflectEnumerateInputVariables(&module, &count, nullptr);
		
		std::vector<SpvReflectInterfaceVariable*> inputVars(count);
		result = spvReflectEnumerateInputVariables(&module, &count, inputVars.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t offset = 0;
		for (size_t i = 0; i < inputVars.size(); ++i)
		{
			const SpvReflectInterfaceVariable& refl_var = *(inputVars[i]);
			if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
			{
				continue;
			}

			if (refl_var.format == SPV_REFLECT_FORMAT_R32G32B32_SFLOAT)
			{
				outVertexInputs.emplace_back(EDataFormat::R32G32B32, offset);
				offset += 12;
			}
			else if (refl_var.format == SPV_REFLECT_FORMAT_R32G32_SFLOAT)
			{
				outVertexInputs.emplace_back(EDataFormat::R32G32, offset);
				offset += 8;
			}
		}

		outVertexInputsSize = offset;
	}

	void fillShaderParameterFromSpvBlockVariable(const SpvReflectBlockVariable& inBlock, uint32_t& outCumulatedOffset, ShaderParameter& outParameter)
	{
		switch (inBlock.type_description->op)
		{
			case SpvOpTypeMatrix:
			{
				outParameter = { EShaderParameterType::MAT4, inBlock.name, outCumulatedOffset };
				outCumulatedOffset += sizeof(glm::mat4);
				break;
			}
			case SpvOpTypeVector:
			{
				if (inBlock.numeric.vector.component_count == 4)
				{
					outParameter = { EShaderParameterType::VEC4, inBlock.name, outCumulatedOffset };
					outCumulatedOffset += sizeof(glm::vec4);
					break;
				}
				else if (inBlock.numeric.vector.component_count == 3)
				{
					outParameter = { EShaderParameterType::VEC3, inBlock.name, outCumulatedOffset };
					outCumulatedOffset += sizeof(glm::vec3);
					break;
				}
				else
				{
					assert(false && "Unhandled SpvOpType");
					break;
				}
			}
			case SpvOpTypeFloat:
			{
				outParameter = { EShaderParameterType::FLOAT, inBlock.name, outCumulatedOffset };
				outCumulatedOffset += sizeof(float);
				break;
			}
			case SpvOpTypeInt:
			{
				outParameter = { EShaderParameterType::UINT, inBlock.name, outCumulatedOffset };
				outCumulatedOffset += sizeof(uint32_t);
				break;
			}
		}
	}

	void extractDescriptorSetBindings(SpvReflectShaderModule& module, Shader& outShader)
	{
		// todo:
		// - extract descriptor sets used
		// - for now, assume pretend cameraBlock / lightBlock are the same on every shader
		// - texture / texture sampler are a combo that are opt-in
		// - material data is the aim for extracting parameters and should fill outShaderParameters

		uint32_t var_count = 0;
		SpvReflectResult result = spvReflectEnumerateDescriptorBindings(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorBinding*> descriptorBindings(var_count);
		result = spvReflectEnumerateDescriptorBindings(&module, &var_count, descriptorBindings.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t offset = 0;
		for (size_t i = 0; i < descriptorBindings.size(); ++i)
		{
			SpvReflectDescriptorBinding& binding = *descriptorBindings[i];
			SpvReflectBlockVariable& block = descriptorBindings[i]->block;
			// todo: fix this > comparison always false ????
			if (binding.name == "cameraData")
			{
				outShader.bCameraBlock = true;
				continue;
			}
			if (binding.name == "lightData")
			{
				outShader.bLightBlock = true;
				continue;
			}
			if (binding.name == "textures" || binding.name == "textureSampler")
			{
				outShader.bTextures = true;
				continue;
			}
			if (binding.name == "materialData")
			{
				outShader.parameters.resize((size_t)block.member_count);

				for (size_t j = 0; j < block.member_count; ++j)
				{
					SpvReflectBlockVariable& member = block.members[j];
					fillShaderParameterFromSpvBlockVariable(member, offset, outShader.parameters[j]);

					std::cout << member.name << " " << member.type_description->type_name << "" << member.offset << std::endl;
				}
			}
		}
	}

	void extractPushConstants(SpvReflectShaderModule& module, std::vector<ShaderParameter>& outPushConstants, uint32_t& outPushConstantsSize)
	{
		uint32_t var_count = 0;
		SpvReflectResult result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		// push constant blocks, so InstanceData
		std::vector<SpvReflectBlockVariable*> constantBlocks(var_count);
		result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, constantBlocks.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (size_t i = 0; i < constantBlocks.size(); ++i)
		{
			SpvReflectBlockVariable* block = constantBlocks[i];

			for (size_t j = 0; j < block->member_count; ++j)
			{
				SpvReflectBlockVariable& member = block->members[j];
				std::cout << member.name << " " << member.offset << std::endl;
			}
		}
		// todo:
		// - idk ? is instance data changing from shader to shader ? not sure yet
		// - how to handle debug primitive shader ?
	}
}

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

	void extractShaderParameters(const std::vector<char>& inBytes, Shader& outShader)
	{
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(inBytes.size(), inBytes.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		
		extractVertexInputs(module, outShader.vertexInputs, outShader.vertexSize);
		extractDescriptorSetBindings(module, outShader);
		extractPushConstants(module, outShader.pushConstants, outShader.pushConstantsSize);

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