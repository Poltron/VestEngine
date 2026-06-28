#pragma once
#include <stdlib.h>

// Marker based allocator
class StackAllocator
{
public:
	StackAllocator(size_t inCapacity)
		: capacity(inCapacity)
	{
		base = (char*)malloc(capacity);
		top = base;
	}

	~StackAllocator()
	{
		free(base);
	}

	char* create(size_t inAllocationSize)
	{
		assert(top < base + capacity);

		char* ptr = top;
		top += inAllocationSize;

		return ptr;
	}

	void clear()
	{
		top = base;
	}

	void resetTo(char* inMarker)
	{
		top = inMarker;
	}

private:
	size_t capacity = 0;

	char* base = nullptr;
	char* top = nullptr;
};