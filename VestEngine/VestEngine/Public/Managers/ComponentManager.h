#pragma once

#include <unordered_map>

#include "Managers/EntityManager.h"
#include "Utils/DenseArray.h"
#include "Utils/PoolAllocator.h"

#define COMPONENT_MAX 100

template<typename T>
class ComponentManager
{
public:
	ComponentManager()
	{
		components.initialize(COMPONENT_MAX);
	}

	T* create(Entity inID)
	{
		T* element = components.add();
		element->entityID = inID;
		lookupTable.emplace(inID, element);
		return element;
	}

	T* at(size_t inIndex)
	{
		return &components.at(inIndex);
	}

	T* get(Entity inID)
	{
		return lookupTable.at(inID);
	}

	T* getData()
	{
		// note : don't like this dereference
		return &components.at(0);
	}

	size_t size()
	{
		return components.size();
	}

	void destroy(Entity inID)
	{
		T* element = lookupTable.at(inID);
		components.remove(element);
		lookupTable.erase(inID);
	}

private:
	DenseArray<T> components;
	std::unordered_map<Entity, T*> lookupTable;
};

