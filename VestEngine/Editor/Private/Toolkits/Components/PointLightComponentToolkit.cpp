#include "Toolkits/Components/PointLightComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/PointLightComponentInspector.h"

bool PointLightComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->pointLightComponents.contains(inEntity);
}

IInspectorElement* PointLightComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new PointLightComponentInspector(inEntity);
}