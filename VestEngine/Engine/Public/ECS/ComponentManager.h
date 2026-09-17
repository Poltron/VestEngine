#pragma once

#include <string>

#define DEBUG_UI 1
#if DEBUG_UI
#include "imgui.h"
#endif

#include "ECS/EntityManager.h"
#include "Core/Containers/DenseArray.h"

#define INVALID_COMPONENT_INDEX UINT64_MAX

template<typename T>
class ComponentManager
{
public:
	ComponentManager()
		: label(" ")
	{
		components.initialize(ENTITY_MAX);
		memset(lookupTable, UCHAR_MAX, ENTITY_MAX * sizeof(size_t));
	}

	void setLabel(const std::string& inLabel)
	{
		label = inLabel;
	}

	T* create(Entity inID)
	{
		size_t elementIdx = components.add();
		T& element = components.at(elementIdx);
		element.entity = inID;
		
		lookupTable[inID] = elementIdx;
		return &element;
	}

	T* at(size_t inIndex)
	{
		return &components.at(inIndex);
	}

	const T* at(size_t inIndex) const
	{
		return &components.at(inIndex);
	}

	bool contains(Entity inID) const
	{
		return lookupTable[inID] != INVALID_COMPONENT_INDEX;
	}

	T* get(Entity inID)
	{
		size_t idx = lookupTable[inID];
		return components.data() + idx;
	}

	const T* get(Entity inID) const
	{
		size_t idx = lookupTable[inID];
		return components.data() + idx;
	}

	size_t getIndex(Entity inID) const
	{
		return lookupTable[inID];
	}

	T* getData()
	{
		return components.data();
	}

	size_t size() const
	{
		return components.size();
	}

	void destroy(Entity inID)
	{
		size_t elementIdx = lookupTable[inID];
		components.remove(elementIdx);
		lookupTable[inID] = INVALID_COMPONENT_INDEX;
	}

	void moveData(DenseArray<T>&& inData)
	{
		components = std::move(inData);
	}

	void rebuildLookupTable()
	{
		for (size_t i = 0; i < components.size() ; ++i)
		{
			const T& component = components.at(i);
			lookupTable[component.entity] = i;
		}
	}

#if DEBUG_UI
	void drawDebug(float inPositionX, float inPositionY)
	{
		if (ImGui::Begin(label.c_str()))
		{
			ImGui::SetWindowPos(ImVec2(inPositionX, inPositionY), ImGuiCond_FirstUseEver);
			ImGui::SetWindowSize(ImVec2(75, 400), ImGuiCond_FirstUseEver);

			if (ImGui::BeginTable(label.c_str(), 2))
			{
				ImGui::TableHeadersRow();
				ImGui::TableNextColumn();
				ImGui::TableHeader("Entity");
				ImGui::TableNextColumn();
				ImGui::TableHeader("Index");

				for (size_t i = 0; i < ENTITY_MAX; ++i)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					std::string first = std::to_string(i);
					ImGuiTextCentered(first.c_str());

					ImGui::TableNextColumn();
					std::string second = std::to_string(lookupTable[i]);
					ImGuiTextCentered(second.c_str());
				}
				ImGui::EndTable();
			}

			ImGui::End();
		}
	}
#endif // DEBUG_UI

private:
	DenseArray<T> components;
	size_t lookupTable[ENTITY_MAX];
	std::string label;

#if DEBUG_UI
	void ImGuiTextCentered(const char* text)
	{
		float windowWidth = ImGui::GetContentRegionAvail().x;
		float textWidth = ImGui::CalcTextSize(text).x;

		// Center cursor within the cell
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (windowWidth - textWidth) * 0.5f);
		ImGui::TextUnformatted(text);
	}
#endif // DEBUG_UI
};

