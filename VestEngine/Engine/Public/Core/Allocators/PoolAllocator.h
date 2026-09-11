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

	~PoolAllocator()
	{
		free(buffer);
	}

	void Initialize(size_t inCapacity)
	{
		if (inCapacity > 0)
		{
			capacity = inCapacity;

			buffer = (Slot*)malloc(inCapacity * sizeof(T));

			for (size_t i = 0; i < inCapacity - 1; ++i)
			{
				buffer[i].nextSlot = &buffer[i + 1];
			}

			buffer[inCapacity - 1].nextSlot = nullptr;

			head = &buffer[0];
		}
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
	Slot* buffer = nullptr;
	Slot* head = nullptr;

	size_t capacity = 0;
};