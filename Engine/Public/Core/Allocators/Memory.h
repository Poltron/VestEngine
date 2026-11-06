#pragma once

namespace memory
{
	uint32_t getPaddedSize(uint32_t inSize, uint32_t inAlignment);
	void* malloc(size_t inSize, size_t inAlignment);
}