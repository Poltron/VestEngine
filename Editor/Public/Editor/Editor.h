#pragma once

#include "Widgets/EntitySelector.h"
#include "Widgets/Inspector.h"

class Editor;

namespace editor
{
	Editor* getEditor();

	bool initialize();
	void launch();
	void shutdown();
}

class Editor
{
	Inspector inspector;
	EntitySelector entitySelector;

public:
	Inspector& getInspector() { return inspector; }
	EntitySelector& getEntitySelector() { return entitySelector; }
};