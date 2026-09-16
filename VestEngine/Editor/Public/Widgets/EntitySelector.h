#pragma once

#include "ECS/Entity.h"

class EntitySelector
{
	Entity selectedEntity;

public:
	void initialize();
	void update();
	void selectEntity(Entity entity);
};