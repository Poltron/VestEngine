#pragma once

#include "Widgets/InspectorElement.h"

class HierarchyComponentInspector : public IInspectorElement
{
	Entity parent = 0;
	Entity firstChild = 0;
	Entity nextSibling = 0;

public:
	HierarchyComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};