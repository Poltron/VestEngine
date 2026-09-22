#include "Editor/Editor.h"
#include "DemoScene.h"
#include "Core/Engine.h"
#include "Platform/Platform.h"
#include "Render/Renderer.h"
#include "UI/UIManager.h"
#include "UnitTest.h"

int main(int argc, char* argv[])
{
	if (platform::initialize()
		&& engine::initialize()
		&& render::initialize()
		&& ui::initialize())
	{
		test::run();

		if (editor::initialize())
		{
			render::getRenderer()->loadDefaultShaders();

			Scene* scene = engine::createScene();

			if (argc == 3)
			{
				int totalAnimal = atoi(argv[1]);
				int rowSize = atoi(argv[2]);

				demoScene::loadAnimals(*scene, totalAnimal, rowSize);
			}
			else
			{
				demoScene::loadCubesDemo(*scene);
			}

			engine::launch();
		}
	}

	editor::shutdown();
	ui::shutdown();
	engine::shutdown();
	render::shutdown();
	platform::shutdown();
	return 0;
}