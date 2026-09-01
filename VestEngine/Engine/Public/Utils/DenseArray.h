#pragma once
#include <stdlib.h>
#include <cassert>

template <typename T>
class DenseArray
{
public:
	DenseArray()
		: capacity(0), maxIndex(0), buffer(nullptr)
	{}

	void initialize(size_t inCapacity)
	{
		free(buffer);

		capacity = inCapacity;
		if (capacity <= 0)
		{
			return;
		}

		buffer = (T*)malloc(capacity * sizeof(T));
	}

	size_t size() const
	{
		return maxIndex;
	}

	size_t getCapacity() const
	{
		return capacity;
	}

	T* add()
	{
		assert(maxIndex < capacity);
		T* element = new (buffer + maxIndex) T();
		maxIndex++;
		return element;
	}

	void remove(T* inElement)
	{
		memcpy((void*)inElement, (void*)(buffer + maxIndex - 1), sizeof(T));
		maxIndex--;
	}

	T& at(size_t inIndex)
	{
		assert(inIndex < maxIndex);
		return *(buffer + inIndex);
	}

	const T& at(size_t inIndex) const
	{
		assert(inIndex < maxIndex);
		return *(buffer + inIndex);
	}

	T& last()
	{
		return *(buffer + maxIndex - 1);
	}

private:
	T* buffer = nullptr;
	size_t maxIndex = 0;

	size_t capacity = 0;
};