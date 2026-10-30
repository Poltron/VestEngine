#pragma once
#include <cassert>

// Marker based allocator
class StackAllocator
{
public:
	StackAllocator(size_t inCapacity)
		: capacity(inCapacity), top(0)
	{
		base = (uint8_t*)malloc(capacity);
	}

	~StackAllocator()
	{
		free(base);
	}

	size_t create(size_t inAllocationSize)
	{
		assert(false);
		assert(base + top + inAllocationSize < base + capacity);

		size_t newAllocOffset = top;
		top += inAllocationSize;
		return newAllocOffset;
	}

	void clear()
	{
		top = 0;
	}

	void resetTo(size_t inMarker)
	{
		top = inMarker;
	}

	uint8_t* get(size_t inOffset)
	{
		return base + inOffset;
	}

private:
	size_t capacity = 0;

	uint8_t* base = nullptr;
	size_t top = 0;
};