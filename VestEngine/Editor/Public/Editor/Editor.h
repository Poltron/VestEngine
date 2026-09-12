#pragma once

#include "Widgets/Inspector.h"

class Editor;
class Inspector;

namespace editor
{
	Editor* getEditor();

	bool initialize();
	void launch();
	void shutdown();

	void loadDemoScene();
}

class Editor
{
	Inspector inspector;

public:
	Inspector& getInspector() { return inspector; }
};