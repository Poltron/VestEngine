#pragma once

#include <array>
#include <queue>

#include "glm/fwd.hpp"

using Entity = glm::uint32_t;

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
	size_t entityCount = 0;
};