#include "Toolkits/Components/HierarchyComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/HierarchyComponentInspector.h"

bool HierarchyComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->getHierarchyComponents().contains(inEntity);
}

IInspectorElement* HierarchyComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new HierarchyComponentInspector(inEntity);
}