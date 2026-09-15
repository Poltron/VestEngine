#pragma once

#include "Widgets/InspectorElement.h"

class MeshRendererComponentInspector : public IInspectorElement
{
	unsigned int model = 0;
	unsigned int shader = 0;
	bool bOutline = false;

public:
	MeshRendererComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};