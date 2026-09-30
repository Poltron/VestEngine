#include "Widgets/Components/LocalTransformComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Maths.h"
#include "Core/Scene.h"


std::string LocalTransformComponentInspector::getLabel()
{
	return "Local Transform";
}

void LocalTransformComponentInspector::update()
{
	LocalTransformComponent* localTransform = engine::getScene()->getLocalTransformComponents().get(entity);
	if (!localTransform)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	position = maths::getMatrixTranslation(localTransform->computeModel());
	rotation = glm::degrees(glm::eulerAngles(localTransform->getRotation()));
	scale = maths::getMatrixScale(localTransform->computeModel());

	if (ImGui::InputFloat3("Position", &position.x))
	{
		localTransform->setLocalPosition(position);
	}

	if (ImGui::InputFloat3("Rotation", &rotation.x))
	{
		localTransform->setLocalRotation(glm::quat(glm::radians(rotation)));
	}

	if (ImGui::InputFloat3("Scale", &scale.x))
	{
		localTransform->setLocalScale(scale);
	}
}