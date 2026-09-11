#include "Editor/Editor.h"
#include "Core/Engine.h"
#include "Platform/Platform.h"
#include "Render/Renderer.h"
#include "UI/UIManager.h"

int main()
{
	if (platform::initialize()
		&& render::initialize()
		&& engine::initialize()
		&& ui::initialize()
		&& editor::initialize())
	{
		editor::loadDemoScene();
		editor::launch();
	}

	editor::shutdown();
	ui::shutdown();
	engine::shutdown();
	render::shutdown();
	platform::shutdown();
	return 0;
}