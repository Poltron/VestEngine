#include "ECS/EntityManager.h"

#include "Core/Ensure.h"

#include <iostream>

#define ENTITY_ALIVE 1
#define ENTITY_DEAD 0

EntityManager::EntityManager()
{
	numberEntityAlive = 0;

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
	numberEntityAlive++;

	std::cout << "New entity " << entity << std::endl;

	return entity;
}

void EntityManager::markEntityForDestroy(Entity entity)
{
	pendingDestroy.push_back(entity);
}

void EntityManager::destroyMarkedEntities()
{
	for (Entity entity : pendingDestroy)
	{
		destroyEntity(entity);
	}

	pendingDestroy.clear();
}

void EntityManager::destroyEntity(Entity inEntity)
{
	entities[inEntity] = ENTITY_DEAD;
	numberEntityAlive--;

	std::cout << "Release entity " << inEntity << std::endl;
	availableIds.push(inEntity);
}

bool EntityManager::exists(Entity inEntity) const
{
	return EntityFuncs::isEntityValid(inEntity) && entities[inEntity] == ENTITY_ALIVE;
}