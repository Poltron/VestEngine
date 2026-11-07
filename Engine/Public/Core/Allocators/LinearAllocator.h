#pragma once

#include <cassert>

#include "Core/Allocators/Memory.h"

class LinearAllocator
{
	size_t capacity = 0;
	size_t alignment = 0;

	uint8_t* base = nullptr;
	size_t top = 0;

public:
	size_t size() const { return top; }

	LinearAllocator()
		: capacity(0), alignment(0), top(0), base(nullptr)
	{}

	~LinearAllocator()
	{
		if (base != nullptr)
		{
			free(base);
		}
	}

	LinearAllocator(const LinearAllocator& inOther) = delete;
	LinearAllocator& operator=(const LinearAllocator& inOther) = delete;

	LinearAllocator(LinearAllocator&& inOther) noexcept
		: capacity(inOther.capacity), alignment(0), top(inOther.top), base(inOther.base)
	{
		inOther.capacity = 0;
		inOther.alignment = 0;
		inOther.top = 0;
		inOther.base = 0;
	}

	LinearAllocator& operator=(LinearAllocator&& inOther) noexcept
	{
		if (this != &inOther)
		{
			capacity = inOther.capacity;
			alignment = inOther.alignment;
			top = inOther.top;
			base = inOther.base;

			inOther.capacity = 0;
			inOther.alignment = 0;
			inOther.top = 0;
			inOther.base = nullptr;
		}
		return *this;
	}

	void reserve(size_t inCapacity, size_t inAlignment = 1)
	{
		uint8_t* newBase = (uint8_t*)std::malloc(inCapacity);

		if (base != nullptr)
		{
			memcpy(newBase, base, capacity);
			free(base);
		}

		base = newBase;
		capacity = inCapacity;
		alignment = inAlignment;
	}

	size_t create(size_t inAllocationSize)
	{
		assert(top + inAllocationSize < capacity);

		size_t newAllocOffset = top;
		top += inAllocationSize;
		return newAllocOffset;
	}

	void fill(size_t inOffset, const void* inData, size_t inSize)
	{
		assert(inOffset + inSize < capacity);
		memcpy(get(inOffset), inData, inSize);
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
};