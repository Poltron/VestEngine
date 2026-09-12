#include "Toolkits/Components/DirectionalLightComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/DirectionalLightComponentInspector.h"

bool DirectionalLightComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->directionalLightComponents.contains(inEntity);
}

IInspectorElement* DirectionalLightComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new DirectionalLightComponentInspector(inEntity);
}