#include "Widgets/Components/MeshRendererComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"

std::string MeshRendererComponentInspector::getLabel()
{
	return "Mesh Renderer";
}

void MeshRendererComponentInspector::update()
{
	MeshRendererComponent* meshRenderer = engine::getScene()->getMeshRendererComponents().get(entity);
	if (!meshRenderer)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	model = meshRenderer->model.handle;
	shader = meshRenderer->shader.handle;
	bOutline = meshRenderer->bOutline;

	if (ImGui::InputScalar("Model", ImGuiDataType_U32, &model))
	{
		meshRenderer->model = model;
	}

	if (ImGui::InputScalar("Shader", ImGuiDataType_U32, &shader))
	{
		meshRenderer->shader = shader;
	}

	if (ImGui::Checkbox("Outline", &bOutline))
	{
		meshRenderer->bOutline = bOutline;
	}
}