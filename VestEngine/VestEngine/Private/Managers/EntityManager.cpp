#include "Managers/EntityManager.h"

#include <iostream>

EntityManager::EntityManager()
	: entityCount(0), entities()
{
	for (Entity id = 0; id < MAX_ENTITIES; ++id)
	{
		availableIds.push(id);
	}
}

Entity EntityManager::createEntity()
{
	Entity entity = availableIds.front();
	availableIds.pop();
	++entityCount;

	std::cout << "New entity " << entity << std::endl;

	return entity;
}

void EntityManager::destroyEntity(Entity inEntity)
{
	availableIds.push(inEntity);
	--entityCount;

	std::cout << "Release entity " << inEntity << std::endl;
}