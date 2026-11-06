#pragma once

#include <glm/mat4x4.hpp>
#include <stdint.h>

#define GPURESOURCE_INVALID_ID UINT32_MAX

struct GPUAllocHandle
{
	uint32_t id;

	GPUAllocHandle()
		: id(invalid.id)
	{}

	GPUAllocHandle(uint32_t inId)
		: id(inId)
	{}

	bool isValid() const
	{
		return id != GPURESOURCE_INVALID_ID;
	}

	static GPUAllocHandle invalid;
};

using GPUProxyHandle = GPUAllocHandle;

struct BufferHandle
{
	uint32_t handle;
	uint32_t offset;
	uint32_t size;

	BufferHandle()
		: BufferHandle(invalid.handle, invalid.offset, invalid.size)
	{}

	BufferHandle(uint32_t inHandle, uint32_t inOffset, uint32_t inSize)
		: handle(inHandle), offset(inOffset), size(inSize)
	{}

	bool isValid() const
	{
		return size != 0;
	}

	static BufferHandle invalid;
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