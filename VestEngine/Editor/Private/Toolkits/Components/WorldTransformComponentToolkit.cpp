#include "Toolkits/Components/WorldTransformComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/WorldTransformComponentInspector.h"

bool WorldTransformComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->worldTransformComponents.contains(inEntity);
}

IInspectorElement* WorldTransformComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new WorldTransformComponentInspector(inEntity);
}

