#pragma once

#include <functional>

#include "Core/Ensure.h"
#include "Render/GPUHandle.h"
#include "Render/Vertex.h"
#include "Render/Resources/TextureProxy.h"

struct alignas(16) CameraData
{
	glm::mat4 view;
	glm::mat4 proj;
};

struct alignas(128) TextureData
{
	 TextureView textures[16];
};

struct alignas(16) InstanceData
{
	glm::mat4 model;
};

// todo: should not exist, this should be dynamic on the material side
// UNLIT SHADER
struct UnlitMaterialData
{
	glm::vec4 color;
	uint32_t diffuseTexIndex;
};

// LIT SHADER
struct AmbientLight
{
	glm::vec3 color; // 12
	float intensity; // 16
};

struct DirectionalLight
{
	glm::vec3 color; // 12
	glm::vec3 forward; // 24
	float intensity; // 28
};

struct alignas(16) PointLight
{
	glm::vec3 color; // 12
	glm::vec3 position; // 24
	float intensity; // 28
	float constant; // 32
	float linear; // 34
	float quadratic; // 38

	char padding[10]; // 48
};

struct LightData
{
	AmbientLight ambient;
	DirectionalLight directional;
	PointLight points[4];
	uint32_t pointLightCount;
	glm::vec3 viewPosition;
};

struct LitMaterialData
{
	glm::vec4 color;
	uint32_t diffuseTexIndex;
	uint32_t specularTexIndex;
	float shininess;
};

enum EShaderStageFlagBits : uint8_t
{
	NONE = 0x00,
	VERTEX = 0x01,
	FRAGMENT = 0x02
};

struct ShaderStageInfo
{
	EShaderStageFlagBits stage;
	VkShaderModule module;
};
typedef uint8_t ShaderStageFlags;

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
	uint32_t size;
	std::vector<ShaderParameter> parameters;

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
		for (const ShaderParameter& parameter : parameters)
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

struct VertexInputAttribute
{
	EDataFormat format;
	uint32_t offset;

	VertexInputAttribute(EDataFormat inFormat, uint32_t inOffset)
		: format(inFormat), offset(inOffset)
	{ }
};

#ifdef RENDER_VULKAN
typedef VkShaderModule CompiledShaderHandle;
#elif defined(RENDER_OPENGL)
typedef GLuint CompiledShaderHandle;
#endif

struct Shader
{
	std::string_view path;
	CompiledShaderHandle handle;

	std::vector<VertexInputAttribute> vertexInputs;
	std::vector<ShaderParameter> parameters;
	std::vector<ShaderParameter> pushConstants;
	uint32_t vertexSize;
	uint32_t parametersSize;
	uint32_t pushConstantsSize;

	EShaderStageFlagBits stage;
	bool bCameraBlock;
	bool bLightBlock;
	bool bTextures;
};

namespace shader
{
	inline ShaderParameters getUnlitMaterialShaderParameters()
	{
		ShaderParameters parameters;
		parameters.size = sizeof(glm::vec4) + sizeof(uint32_t);
		parameters.parameters = 
		{ 
			{ EShaderParameterType::VEC4, "color", 0 },
			{ EShaderParameterType::UINT, "diffuse", sizeof(uint32_t) }
		};

		return parameters;
	}

	inline std::vector<VertexInputAttribute> getUnlitMaterialVertexInputs()
	{
		const std::vector<VertexInputAttribute> vertexInputAttributes =
		{
			{ EDataFormat::R32G32B32, offsetof(Vertex, position) },
			{ EDataFormat::R32G32B32, offsetof(Vertex, normal) },
			{ EDataFormat::R32G32, offsetof(Vertex, texCoords) }
		};
		return vertexInputAttributes;
	}

	inline uint32_t getUnlitMaterialVertexSize()
	{
		return sizeof(Vertex);
	}

	//
	inline ShaderParameters getLitMaterialShaderParameters()
	{
		ShaderParameters parameters;
		parameters.size = sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float);
		parameters.parameters =
		{
			{ EShaderParameterType::VEC4, "color", 0 },
			{ EShaderParameterType::UINT, "diffuse", sizeof(uint32_t) },
			{ EShaderParameterType::UINT, "specular", sizeof(uint32_t) },
			{ EShaderParameterType::FLOAT, "shininess", sizeof(float) }
		};

		return parameters;
	}

	inline std::vector<VertexInputAttribute> getLitMaterialVertexInputs()
	{
		const std::vector<VertexInputAttribute> vertexInputAttributes =
		{
			{ EDataFormat::R32G32B32, offsetof(Vertex, position) },
			{ EDataFormat::R32G32B32, offsetof(Vertex, normal) },
			{ EDataFormat::R32G32, offsetof(Vertex, texCoords) }
		};
		return vertexInputAttributes;
	}

	inline uint32_t getLitMaterialVertexSize()
	{
		return sizeof(Vertex);
	}
}