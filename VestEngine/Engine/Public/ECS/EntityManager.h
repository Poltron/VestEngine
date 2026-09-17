#pragma once

#include <queue>

#include "ECS/Entity.h"

#define ENTITY_MAX 100

class EntityManager
{
public:
	EntityManager();

	Entity createEntity();
	void destroyEntity(Entity inEntity);
	bool exists(Entity inEntity);

private:
	std::queue<Entity> availableIds;
	Entity entities[ENTITY_MAX];
};