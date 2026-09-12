#include "Widgets/Components/HierarchyComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"

std::string HierarchyComponentInspector::getLabel()
{
	return "Hierarchy";
}

void HierarchyComponentInspector::update()
{
	HierarchyComponent* hierarchy = engine::getScene()->hierarchyComponents.get(entity);
	if (!hierarchy)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	parent = hierarchy->parent;
	firstChild = hierarchy->firstChild;
	nextSibling = hierarchy->nextSibling;

	if (ImGui::InputScalar("Parent", ImGuiDataType_U32, &parent))
	{
		hierarchy->parent = parent;
	}

	if (ImGui::InputScalar("First Child", ImGuiDataType_U32, &firstChild))
	{
		hierarchy->firstChild = firstChild;
	}

	if (ImGui::InputScalar("Next Sibling", ImGuiDataType_U32, &nextSibling))
	{
		hierarchy->nextSibling = nextSibling;
	}
}