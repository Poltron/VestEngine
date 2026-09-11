#pragma once

class Editor
{};

namespace editor
{
	Editor* getEditor();

	bool initialize();
	void launch();
	void shutdown();

	void loadDemoScene();
}