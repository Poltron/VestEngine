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
	materialInstance = meshRenderer->materialInstance.handle;
	//bOutline = meshRenderer->bOutline;

	if (ImGui::InputScalar("Model", ImGuiDataType_U32, &model))
	{
		meshRenderer->model = ResourceHandle(model);
	}

	if (ImGui::InputScalar("Material", ImGuiDataType_U32, &materialInstance))
	{
		meshRenderer->materialInstance = ResourceHandle(materialInstance);
	}

	//if (ImGui::Checkbox("Outline", &bOutline))
	//{
	//	meshRenderer->bOutline = bOutline;
	//}
}