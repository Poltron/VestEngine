#pragma once

#include "Utils/Ensure.h"

template <typename T>
class DenseArray
{
public:
	DenseArray()
		: capacity(0), maxIndex(0), buffer(nullptr)
	{}

	void initialize(size_t inCapacity)
	{
		clear();

		if (inCapacity == 0)
		{
			return;
		}

		capacity = inCapacity;
		buffer = (T*)malloc(capacity * sizeof(T));
	}

	void clear()
	{
		free(buffer);
		buffer = nullptr;
		capacity = 0;
		maxIndex = 0;
	}

	T* data()
	{
		return buffer;
	}

	const T* data() const
	{
		return buffer;
	}

	size_t size() const
	{
		return maxIndex;
	}

	size_t getCapacity() const
	{
		return capacity;
	}

	size_t add()
	{
		ensure(maxIndex < capacity);
		T* element = new (buffer + maxIndex) T();
		size_t elemIndex = maxIndex;
		maxIndex++;
		return elemIndex;
	}

	void remove(size_t inElementIndex)
	{
		ensure(inElementIndex < maxIndex);
		T* element = buffer + inElementIndex;

		ensure(buffer != nullptr);
		ensure(element != nullptr);
		ensure(element >= buffer);

		T* lastElement = buffer + maxIndex - 1;
		ensure((void*)element <= (void*)lastElement);

		memcpy((void*)element, (void*)lastElement, sizeof(T));
		maxIndex--;
	}

	T& at(size_t inIndex)
	{
		ensure(inIndex < maxIndex);
		return *(buffer + inIndex);
	}

	const T& at(size_t inIndex) const
	{
		ensure(inIndex < maxIndex);
		return *(buffer + inIndex);
	}

	T& last()
	{
		ensure(buffer != nullptr);
		ensure(capacity > 0);
		ensure(maxIndex > 0);
		return *(buffer + maxIndex - 1);
	}

private:
	T* buffer = nullptr;
	size_t maxIndex = 0;
	size_t capacity = 0;
};