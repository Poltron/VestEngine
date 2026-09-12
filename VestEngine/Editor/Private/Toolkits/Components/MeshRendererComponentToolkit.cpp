#include "Toolkits/Components/MeshRendererComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/MeshRendererComponentInspector.h"

bool MeshRendererComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->meshRendererComponents.contains(inEntity);
}

IInspectorElement* MeshRendererComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new MeshRendererComponentInspector(inEntity);
}

