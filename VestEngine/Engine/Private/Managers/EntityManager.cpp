#include "Managers/EntityManager.h"

#include <iostream>

EntityManager::EntityManager()
	: entities()
{
	for (Entity id = 1; id < MAX_ENTITIES; ++id)
	{
		availableIds.push(id);
	}
}

Entity EntityManager::createEntity()
{
	Entity entity = availableIds.front();
	availableIds.pop();

	std::cout << "New entity " << entity << std::endl;

	return entity;
}

void EntityManager::destroyEntity(Entity inEntity)
{
	availableIds.push(inEntity);

	std::cout << "Release entity " << inEntity << std::endl;
}