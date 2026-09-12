#pragma once

#include "Widgets/InspectorElement.h"

class WorldTransformComponentInspector : public IInspectorElement
{
public:
	WorldTransformComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};