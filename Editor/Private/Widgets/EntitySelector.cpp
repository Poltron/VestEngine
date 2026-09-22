#include "Widgets/EntitySelector.h"

#include "imgui.h"

#include "Editor/Editor.h"
#include "Widgets/Inspector.h"

void EntitySelector::initialize()
{}

void EntitySelector::update()
{
	if (ImGui::Begin("Entity Selector", 0, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize({ 200, 60 });
		if (ImGui::InputScalar("ID", ImGuiDataType_U32, &selectedEntity))
		{
			editor::getEditor()->getInspector().show(selectedEntity);
		}

		ImGui::End();
	}
}

void EntitySelector::selectEntity(Entity entity)
{
	selectedEntity = entity;
	editor::getEditor()->getInspector().show(selectedEntity);
}