#include "ECS/EntityManager.h"

#include <iostream>

EntityManager::EntityManager()
{
	entities[0] = 0;

	for (Entity id = 1; id < ENTITY_MAX; ++id)
	{
		availableIds.push(id);
		entities[id] = 0;
	}
}

Entity EntityManager::createEntity()
{
	Entity entity = availableIds.front();
	availableIds.pop();

	entities[entity] = 1;

	std::cout << "New entity " << entity << std::endl;

	return entity;
}

void EntityManager::destroyEntity(Entity inEntity)
{
	entities[inEntity] = 0;
	availableIds.push(inEntity);

	std::cout << "Release entity " << inEntity << std::endl;
}

bool EntityManager::exists(Entity inEntity) const
{
	return (entities[inEntity] == 1);
}