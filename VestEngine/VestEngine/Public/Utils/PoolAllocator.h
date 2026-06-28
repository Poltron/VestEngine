#pragma once
#include <stdlib.h>

// Fixed-size block allocator
template <typename T>
class PoolAllocator
{
public:
	union Slot
	{
		T element;
		Slot* nextSlot;
	};

	PoolAllocator(size_t inMaxElementNumber)
		: maxElementNumber(inMaxElementNumber)
	{
		buffer = (Slot*)malloc(inMaxElementNumber * sizeof(T));

		for (size_t i = 0; i < inMaxElementNumber - 1; ++i)
		{
			buffer[i].nextSlot = &buffer[i + 1];
		}

		buffer[inMaxElementNumber - 1].nextSlot = nullptr;

		head = &buffer[0];
	}

	~PoolAllocator()
	{
		free(buffer);
	}

	T* Allocate()
	{
		Slot* allocatedSlot = head;
		head = head->nextSlot;
		return &allocatedSlot->element;
	}

	void Deallocate(T* inElement)
	{
		Slot* releasedSlot = reinterpret_cast<Slot*>(inElement);
		releasedSlot->nextSlot = head;
		head->nextSlot = releasedSlot;
	}

private:
	Slot* buffer;
	Slot* head;

	size_t maxElementNumber;
};