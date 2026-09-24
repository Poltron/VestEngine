#pragma once

#include "Widgets/EntitySelector.h"
#include "Widgets/Inspector.h"
#include "Widgets/RendererInfo.h"

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
	RendererInfo rendererInfo;

public:
	Inspector& getInspector() { return inspector; }
	EntitySelector& getEntitySelector() { return entitySelector; }
	RendererInfo& getRendererInfo() { return rendererInfo; }
};