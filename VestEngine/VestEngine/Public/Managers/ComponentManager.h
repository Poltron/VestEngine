#pragma once

#include <unordered_map>

#include "Managers/EntityManager.h"
#include "Utils/DenseArray.h"

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
		element->entity = inID;
		lookupTable.emplace(inID, element);
		return element;
	}

	T* at(size_t inIndex)
	{
		return &components.at(inIndex);
	}

	const T* at(size_t inIndex) const
	{
		return &components.at(inIndex);
	}

	bool contains(Entity inID) const
	{
		auto pair = lookupTable.find(inID);
		return pair != lookupTable.end();
	}

	T* get(Entity inID)
	{
		return lookupTable.at(inID);
	}

	const T* get(Entity inID) const
	{
		return lookupTable.at(inID);
	}

	T* getData()
	{
		// note : don't like this dereference
		return &components.at(0);
	}

	size_t size() const
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

