#include "Widgets/Inspector.h"

#include "imgui.h"

#include "Core/Ensure.h"
#include "Toolkits/Components/LocalTransformComponentToolkit.h"
#include "Toolkits/Components/WorldTransformComponentToolkit.h"
#include "Toolkits/Components/HierarchyComponentToolkit.h"
#include "Toolkits/Components/MeshRendererComponentToolkit.h"
#include "Toolkits/Components/RigidbodyComponentToolkit.h"
#include "Toolkits/Components/DirectionalLightComponentToolkit.h"
#include "Toolkits/Components/PointLightComponentToolkit.h"
#include "Toolkits/IToolkit.h"
#include "Widgets/InspectorElement.h"

void Inspector::initialize()
{
	registerToolkit<LocalTransformComponentToolkit>();
	registerToolkit<WorldTransformComponentToolkit>();
	registerToolkit<HierarchyComponentToolkit>();
	registerToolkit<MeshRendererComponentToolkit>();
	registerToolkit<RigidbodyComponentToolkit>();
	registerToolkit<DirectionalLightComponentToolkit>();
	registerToolkit<PointLightComponentToolkit>();
}

void Inspector::show(Entity inEntity)
{
	watchedEntity = inEntity;
	inspectorElements.clear();

	for (IToolkit* toolkit : toolkits)
	{
		ensure(toolkit);

		if (toolkit->shouldCreateInspectorElement(inEntity))
		{
			IInspectorElement* element = toolkit->createInspectorElement(inEntity);
			inspectorElements.push_back(element);
		}
	}
}

void Inspector::update()
{
	if (!EntityFuncs::isEntityValid(watchedEntity))
	{
		return;
	}

	std::string inspectorLabel = "Entity ";
	inspectorLabel.append(std::to_string(watchedEntity).c_str());
	inspectorLabel.append("###Inspector");
	
	if (ImGui::Begin(inspectorLabel.c_str()))
	{
		for (const auto& currentInspector : inspectorElements)
		{
			const std::string sectionLabel = currentInspector->getLabel().size() ? currentInspector->getLabel() : " ";
			if (ImGui::CollapsingHeader(sectionLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				currentInspector->update();
			}
		}

		ImGui::End();
	}
}

IToolkit* Inspector::findToolkit(size_t inTypeID)
{
	ensure(toolkits.size() >= inTypeID);
	ensure(toolkits[inTypeID] != nullptr);

	return toolkits[inTypeID];
}