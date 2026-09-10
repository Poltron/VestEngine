#pragma once

class Editor
{};

namespace editor
{
	Editor* getEditor();

	bool initialize();
	void loadDemoScene();
	void shutdown();
}