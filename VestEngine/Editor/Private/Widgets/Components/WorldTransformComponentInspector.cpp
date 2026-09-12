#include "Widgets/Components/WorldTransformComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"

std::string WorldTransformComponentInspector::getLabel()
{
	return "World Transform";
}

void WorldTransformComponentInspector::update()
{
	WorldTransformComponent* worldTransform = engine::getScene()->worldTransformComponents.get(entity);
	if (!worldTransform)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	ImGui::InputFloat4("Mat[0]", &worldTransform->model[0].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat4("Mat[1]", &worldTransform->model[1].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat4("Mat[2]", &worldTransform->model[2].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat4("Mat[3]", &worldTransform->model[3].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
}