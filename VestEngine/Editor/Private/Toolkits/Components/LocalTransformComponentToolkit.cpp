#include "Toolkits/Components/LocalTransformComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/LocalTransformComponentInspector.h"

bool LocalTransformComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->localTransformComponents.contains(inEntity);
}

IInspectorElement* LocalTransformComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new LocalTransformComponentInspector(inEntity);
}