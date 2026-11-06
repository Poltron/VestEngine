#pragma once

#ifdef TRACY_ENABLE
#include "Core/Ensure.h"
#include "tracy/Tracy.hpp"

namespace memory
{
	uint32_t getPaddedSize(uint32_t inSize, uint32_t inAlignment)
	{
		ensure(inAlignment > 0);
		return (inSize + inAlignment - 1) & ~(inAlignment - 1);
	}

	void* malloc(size_t inSize, size_t inAlignment)
	{
		uint8_t* ptr = (uint8_t*)std::malloc(inSize);
		ptr += (inAlignment - 1);
		ptr = (uint8_t*)((size_t)ptr & ~(inAlignment - 1));
		return ptr;
	}
}

void* operator new(std::size_t size)
{
	void* ptr = std::malloc(size);
	TracyAlloc(ptr, size);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	std::free(ptr);
}

#endif