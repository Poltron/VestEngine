#pragma once

#include <string>

#include "ECS/Entity.h"

class IInspectorElement
{
protected:
	Entity entity;

public:
	IInspectorElement(Entity inEntity)
		: entity(inEntity)
	{ }

	virtual void update() = 0;
	virtual std::string getLabel() = 0;
};