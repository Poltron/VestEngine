#pragma once

#include <queue>

#include "ECS/Entity.h"

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