#pragma once

#include "Core/ResourceHandle.h"
#include "Widgets/InspectorElement.h"

class MeshRendererComponentInspector : public IInspectorElement
{
	ResourceHandle model = 0;
	ResourceHandle materialInstance = 0;
	//bool bOutline = false;

public:
	MeshRendererComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};