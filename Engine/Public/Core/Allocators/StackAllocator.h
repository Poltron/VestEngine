#pragma once
#include <cassert>

// Marker based allocator
class StackAllocator
{
public:
	StackAllocator()
		: capacity(0), top(0), base(nullptr)
	{}

	~StackAllocator()
	{
		free(base);
	}

	// todo: copy/move ctr

	void reserve(size_t inCapacity)
	{
		uint8_t* newBase = (uint8_t*)malloc(capacity);

		if (base != nullptr)
		{
			memcpy(newBase, base, capacity);
			free(base);
		}

		base = newBase;
		capacity = inCapacity;
	}

	size_t create(size_t inAllocationSize)
	{
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

	const uint8_t* get(size_t inOffset) const
	{
		return base + inOffset;
	}

private:
	size_t capacity = 0;

	uint8_t* base = nullptr;
	size_t top = 0;
};