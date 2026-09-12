#include "Widgets/Components/DirectionalLightComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "ECS/Components/DirectionalLightComponent.h"

std::string DirectionalLightComponentInspector::getLabel()
{
	return "Directional Light";
}

void DirectionalLightComponentInspector::update()
{
	DirectionalLightComponent* directionalLight = engine::getScene()->directionalLightComponents.get(entity);
	if (!directionalLight)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	color = directionalLight->color;
	intensity = directionalLight->intensity;

	if (ImGui::InputFloat3("Color", &color.x))
	{
		directionalLight->color = color;
	}

	if (ImGui::InputFloat3("Intensity", &intensity))
	{
		directionalLight->intensity = intensity;
	}
}