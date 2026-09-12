#pragma once

#include "ECS/Entity.h"

class IInspectorElement;

class IToolkit
{
public:
	virtual bool shouldCreateInspectorElement(Entity inEntity) = 0;
	virtual IInspectorElement* createInspectorElement(Entity inEntity) = 0;
};