#pragma once

#include <queue>

#include "ECS/Entity.h"

#define ENTITY_FIRST 1
#define ENTITY_MAX 10010

class EntityManager
{
public:
	EntityManager();

	Entity createEntity();
	void destroyEntity(Entity inEntity);
	bool exists(Entity inEntity) const;

private:
	std::queue<Entity> availableIds;
	Entity entities[ENTITY_MAX];
};