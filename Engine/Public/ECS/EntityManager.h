#pragma once

#include <queue>

#include "ECS/Entity.h"

class EntityManager
{
	std::queue<Entity> availableIds;
	Entity entities[ENTITY_MAX];
	size_t numberEntityAlive;

	std::vector<Entity> pendingDestroy;
	
public:
	EntityManager();

	Entity createEntity();
	bool exists(Entity inEntity) const;

	size_t getNumberEntityAlive() const { return numberEntityAlive; }

	void markEntityForDestroy(Entity entity);
	void destroyMarkedEntities();
	const std::vector<Entity>& getEntitiesPendingDestruction() const { return pendingDestroy; }

private:
	void destroyEntity(Entity inEntity);
};