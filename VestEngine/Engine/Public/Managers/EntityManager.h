#pragma once

#include <array>
#include <queue>

#include "Components/Entity.h"

const size_t MAX_ENTITIES = 100;

class EntityManager
{
public:
	EntityManager();

	Entity createEntity();
	void destroyEntity(Entity inEntity);

private:
	std::queue<Entity> availableIds;
	Entity entities[MAX_ENTITIES];
};