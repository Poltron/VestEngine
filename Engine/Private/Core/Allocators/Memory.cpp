#pragma once

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"

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