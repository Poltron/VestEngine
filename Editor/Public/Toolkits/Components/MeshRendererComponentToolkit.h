#pragma once

#include "Toolkits/IToolkit.h"

class IInspectorElement;

class MeshRendererComponentToolkit : public IToolkit
{
public:
	virtual bool shouldCreateInspectorElement(Entity inEntity) override;
	virtual IInspectorElement* createInspectorElement(Entity inEntity) override;
};

