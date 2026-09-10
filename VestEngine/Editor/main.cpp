#include "Editor.h"
#include "Engine.h"
#include "Platform/Platform.h"

int main()
{
	platform::initialize();
	renderer::initialize();
	engine::initialize();
	editor::initialize();
	
	editor::loadDemoScene();
	engine::launch();

	editor::shutdown();
	engine::shutdown();
	renderer::shutdown();
	platform::shutdown();
	return 0;
}