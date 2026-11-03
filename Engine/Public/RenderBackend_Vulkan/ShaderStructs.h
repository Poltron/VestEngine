#pragma once

#include <array>
#include <functional>
#include <vulkan/vulkan.h>

#include "Core/Ensure.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"

struct FrameData
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct TextureData
{
	alignas(128) VkImageView textures[16];
};

// todo: should not exist, this should be dynamic on the material side
struct MaterialData
{
	alignas(16) glm::vec4 color;
	alignas(4) uint32_t diffuseTexIndex;
};

struct InstanceData
{
	alignas(16) glm::mat4 model;
};

enum EShaderStageFlagBits : uint8_t
{
	NONE = 0x00,
	VERTEX = 0x01,
	FRAGMENT = 0x02
};

struct ShaderStageInfo
{
	std::string relativePath;
	EShaderStageFlagBits stage;
};
typedef uint8_t ShaderStageFlags;

// todo : vk specific, should be moved 
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

enum class EShaderParameterType : uint8_t
{
	UINT,
	FLOAT,
	VEC3,
	VEC4,
	MAT4
};

struct ShaderParameter
{
	EShaderParameterType type;
	std::string name;
	uint32_t offset;
};

// todo: compute size/offsets automatically depending on datatype
struct ShaderParameters
{
	// note: same order as in shader
	uint32_t size;
	std::vector<ShaderParameter> vertexParameters;
	std::vector<ShaderParameter> fragmentParameters;

	glm::vec4 getVec4(std::string_view inName, const uint8_t* inBuffer) const
	{
		uint32_t offset = getParameterOffset(inName);
		glm::vec4* parameterPtr = (glm::vec4*)(inBuffer + offset);
		return *parameterPtr;
	}

	uint32_t getUint(std::string_view inName, const uint8_t* inBuffer) const
	{
		uint32_t offset = getParameterOffset(inName);
		uint32_t* parameterPtr = (uint32_t*)(inBuffer + offset);
		return *parameterPtr;
	}

	uint32_t getParameterOffset(std::string_view inName) const
	{
		for (const ShaderParameter& parameter : vertexParameters)
		{
			if (parameter.name == inName)
			{
				return parameter.offset;
			}
		}
		
		for (const ShaderParameter& parameter : fragmentParameters)
		{
			if (parameter.name == inName)
			{
				return parameter.offset;
			}
		}

		// note: parameter not found
		ensure(false);
		return UINT32_MAX;
	}
};

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

namespace shader
{
	inline std::vector<ShaderStageInfo> getUnlitMaterialStageInfos()
	{
		const std::vector<ShaderStageInfo> shaderStageInfos =
		{
			{ "Resources/Shaders/Vulkan/default.vert.spv", EShaderStageFlagBits::VERTEX },
			{ "Resources/Shaders/Vulkan/unlit.frag.spv", EShaderStageFlagBits::FRAGMENT }
		};
		return shaderStageInfos;
	}

	inline ShaderParameters getUnlitMaterialShaderParameters()
	{
		ShaderParameters parameters;
		parameters.size = sizeof(glm::vec4) + sizeof(uint32_t);
		parameters.fragmentParameters = 
		{ 
			{ EShaderParameterType::VEC4, "color", 0 },
			{ EShaderParameterType::UINT, "diffuse", sizeof(glm::vec4) }
		};

		return parameters;
	}

	inline std::vector<VertexInputAttributeDesc> getUnlitMaterialVertexInputs()
	{
		const std::vector<VertexInputAttributeDesc> vertexInputAttributeDesc =
		{
			{ EDataFormat::R32G32B32, offsetof(Vertex, position) },
			{ EDataFormat::R32G32B32, offsetof(Vertex, normal) },
			{ EDataFormat::R32G32, offsetof(Vertex, texCoords) }
		};
		return vertexInputAttributeDesc;
	}

	inline uint32_t getUnlitMaterialVertexSize()
	{
		return sizeof(Vertex);
	}

	//
	inline std::vector<ShaderStageInfo> getLitMaterialStageInfos()
	{
		const std::vector<ShaderStageInfo> shaderStageInfos =
		{
			{ "Resources/Shaders/Vulkan/default.vert.spv", EShaderStageFlagBits::VERTEX },
			{ "Resources/Shaders/Vulkan/lit.frag.spv", EShaderStageFlagBits::FRAGMENT }
		};
		return shaderStageInfos;
	}
}