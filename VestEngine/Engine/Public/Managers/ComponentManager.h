#pragma once

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "Managers/EntityManager.h"
#include "Utils/DenseArray.h"

#define COMPONENT_MAX 100

template<typename T>
class ComponentManager
{
public:
	ComponentManager()
	{
		components.initialize(COMPONENT_MAX);
		label = " ";
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
		lookupTable.emplace(inID, elementIdx);
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
		auto pair = lookupTable.find(inID);
		return pair != lookupTable.end();
	}

	T* get(Entity inID)
	{
		size_t idx = lookupTable.at(inID);
		return components.data() + idx;
	}

	const T* get(Entity inID) const
	{
		size_t idx = lookupTable.at(inID);
		return components.data() + idx;
	}

	size_t getIndex(Entity inID) const
	{
		return lookupTable.at(inID);
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
		size_t elementIdx = lookupTable.at(inID);
		components.remove(elementIdx);
		lookupTable.erase(inID);
	}

	void moveData(DenseArray<T>&& inData)
	{
		components = std::move(inData);
	}

	void rebuildLookupTable()
	{
		lookupTable.clear();
		for (size_t i = 0; i < components.size() ; ++i)
		{
			const T& component = components.at(i);
			lookupTable.insert({ component.entity, i});
		}
	}

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

				for (const auto& element : lookupTable)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					std::string first = std::to_string(element.first);
					ImGuiTextCentered(first.c_str());

					ImGui::TableNextColumn();
					std::string second = std::to_string(element.second);
					ImGuiTextCentered(second.c_str());
				}
				ImGui::EndTable();
			}

			ImGui::End();
		}
	}

private:
	DenseArray<T> components;
	// change to a sparse array of entityID to component index
	std::unordered_map<Entity, size_t> lookupTable;
	std::string label;

	void ImGuiTextCentered(const char* text)
	{
		float windowWidth = ImGui::GetContentRegionAvail().x;
		float textWidth = ImGui::CalcTextSize(text).x;

		// Center cursor within the cell
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (windowWidth - textWidth) * 0.5f);
		ImGui::TextUnformatted(text);
	}
};

