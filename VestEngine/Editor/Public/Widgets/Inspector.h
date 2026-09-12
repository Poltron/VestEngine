#pragma once

#include <vector>

#include "ECS/Entity.h"

class IInspectorElement;
class IToolkit;

class Inspector
{
	Entity watchedEntity;
	std::vector<IToolkit*> toolkits;

	std::vector<IInspectorElement*> inspectorElements;

public:
	void initialize();
	void show(Entity inEntity);
	void update();

	template<typename T>
	void registerToolkit();

	IToolkit* findToolkit(size_t inTypeID);
};

// .inl
template<typename T>
void Inspector::registerToolkit()
{
	toolkits.push_back(new T());
}