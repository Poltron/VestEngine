#include "Toolkits/Components/RigidbodyComponentToolkit.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "Widgets/Components/RigidbodyComponentInspector.h"

bool RigidbodyComponentToolkit::shouldCreateInspectorElement(Entity inEntity)
{
	return engine::getScene()->rigidbodyComponents.contains(inEntity);
}

IInspectorElement* RigidbodyComponentToolkit::createInspectorElement(Entity inEntity)
{
	return new RigidbodyComponentInspector(inEntity);
}