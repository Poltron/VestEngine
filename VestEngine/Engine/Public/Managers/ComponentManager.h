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
		size_t elementIdx = components.add();
		T& element = components.at(elementIdx);
		element.entity = inID;
		lookupTable.emplace(inID, elementIdx);
		return &element;
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
		size_t idx = lookupTable.at(inID);
		return components.data() + idx;
	}

	const T* get(Entity inID) const
	{
		size_t idx = lookupTable.at(inID);
		return components.data() + idx;
	}

	T* getData()
	{
		return components.data();
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
	std::unordered_map<Entity, size_t> lookupTable;
};

