#include "Widgets/Components/PointLightComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"

std::string PointLightComponentInspector::getLabel()
{
	return "Point Light";
}

void PointLightComponentInspector::update()
{
	PointLightComponent* pointLight = engine::getScene()->pointLightComponents.get(entity);
	if (!pointLight)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	color = pointLight->color;
	intensity = pointLight->intensity;

	constant = pointLight->constant;
	linear = pointLight->linear;
	quadratic = pointLight->quadratic;

	if (ImGui::InputFloat3("Color", &color.x))
	{
		pointLight->color = color;
	}

	if (ImGui::InputFloat("Intensity", &intensity))
	{
		pointLight->intensity = intensity;
	}

	ImGui::Spacing();

	if (ImGui::InputFloat("Constant", &constant))
	{
		pointLight->constant = constant;
	}

	if (ImGui::InputFloat("Linear", &linear))
	{
		pointLight->linear = linear;
	}

	if (ImGui::InputFloat("Quadratic", &quadratic))
	{
		pointLight->quadratic = quadratic;
	}
}