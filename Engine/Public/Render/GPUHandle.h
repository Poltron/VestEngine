#pragma once

#include <glm/mat4x4.hpp>
#include <stdint.h>

struct GPUHandle
{
	uint32_t id;

	GPUHandle(uint32_t inId)
		: id(inId)
	{}

	GPUHandle()
		: GPUHandle(UINT32_MAX)
	{}

	bool isValid() const
	{
		return id != UINT32_MAX;
	}
};

struct FrameData
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct MaterialData
{
	alignas(4) glm::vec4 color;
};

struct InstanceData
{
	alignas(16) glm::mat4 model;
};

struct UBOHandle
{
	uint32_t offset;
	uint32_t size;

	UBOHandle(uint32_t inOffset, uint32_t inSize)
		: offset(inOffset), size(inSize)
	{}
};

struct DescriptorSetHandle
{
	uint32_t id;
	uint32_t count;

	DescriptorSetHandle(uint32_t inId, uint32_t inCount)
		: id(inId), count(inCount)
	{}

	DescriptorSetHandle()
		: DescriptorSetHandle(UINT32_MAX, 0)
	{}

	bool isValid() const
	{
		return id != UINT32_MAX;
	}
};

enum class EBufferType
{
	VERTEX_ARRAY,
	VERTICES,
	INDICES
};

enum class EBufferUsage
{
	STATIC,
	STREAM,
	DYNAMIC
};

enum class EPrimitiveMode
{
	POINTS,
	LINES,
	TRIANGLES
};

enum class EComparisonFunc : uint8_t
{
	NEVER = 0,
	LESS,
	LEQUAL,
	EQUAL,
	NOTEQUAL,
	GEQUAL,
	GREATER,
	ALWAYS
};

enum class ETextureFormat
{
	RGB,
	RGBA
};