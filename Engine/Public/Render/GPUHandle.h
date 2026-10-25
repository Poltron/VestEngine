#pragma once

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