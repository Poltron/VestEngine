#pragma once

class UIManager
{};

namespace ui
{
	bool initialize();
	void shutdown();
	void startFrame();
	void render();

	UIManager* getUIManager();
}