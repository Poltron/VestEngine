#include "Widgets/Components/LocalTransformComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"


std::string LocalTransformComponentInspector::getLabel()
{
	return "Local Transform";
}

void LocalTransformComponentInspector::update()
{
	LocalTransformComponent* localTransform = engine::getScene()->localTransformComponents.get(entity);
	if (!localTransform)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	position = localTransform->getPosition();
	rotation = localTransform->getRotation();
	scale = localTransform->getScale();

	if (ImGui::InputFloat3("Position", &position.x))
	{
		localTransform->setLocalPosition(position);
	}

	if (ImGui::InputFloat3("Rotation", &rotation.x))
	{
		localTransform->setLocalRotation(rotation);
	}

	if (ImGui::InputFloat3("Scale", &scale.x))
	{
		localTransform->setLocalScale(scale);
	}
}