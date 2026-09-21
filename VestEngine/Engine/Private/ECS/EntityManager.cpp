#include "ECS/EntityManager.h"

#include "Core/Ensure.h"

#include <iostream>

#define ENTITY_ALIVE 1
#define ENTITY_DEAD 0

EntityManager::EntityManager()
{
	entities[0] = ENTITY_DEAD;

	for (Entity id = ENTITY_FIRST; id < ENTITY_MAX; ++id)
	{
		availableIds.push(id);
		entities[id] = ENTITY_DEAD;
	}
}

Entity EntityManager::createEntity()
{
	ensure(availableIds.size() > 0);

	Entity entity = availableIds.front();
	availableIds.pop();

	entities[entity] = ENTITY_ALIVE;

	std::cout << "New entity " << entity << std::endl;

	return entity;
}

void EntityManager::destroyEntity(Entity inEntity)
{
	entities[inEntity] = ENTITY_DEAD;
	availableIds.push(inEntity);

	std::cout << "Release entity " << inEntity << std::endl;
}

bool EntityManager::exists(Entity inEntity) const
{
	return entities[inEntity] == ENTITY_ALIVE;
}