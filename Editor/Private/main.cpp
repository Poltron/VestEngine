#include "tracy/Tracy.hpp"

#include "Editor/Editor.h"
#include "DemoScene.h"
#include "Core/Engine.h"
#include "Platform/Platform.h"
#include "Render/Renderer.h"
#include "UI/UIManager.h"
#include "UnitTest.h"

void waitForTracy()
{
	int timeoutMs = 5000;
	while (!TracyIsConnected && timeoutMs > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		timeoutMs -= 100;
	}
}

int main(int argc, char* argv[])
{
	waitForTracy();

	test::run();

	if (platform::initialize()
		&& engine::initialize()
		&& render::initialize()
		&& ui::initialize())
	{
		if (editor::initialize())
		{
			render::getRenderer()->loadDefaultShaders();

			Scene* scene = engine::createScene();

			demoScene::loadAxis(*scene, 0.5f);

			std::string loadedScene = argv[1];
			if (loadedScene.compare("primitives") == 0)
			{
				demoScene::loadPrimitivesDemo(*scene);
			}
			else if (loadedScene.compare("cubes") == 0)
			{
				demoScene::loadCubesDemo(*scene);
			}
			else if (loadedScene.compare("animals") == 0)
			{
				int totalAnimal = atoi(argv[2]);
				int rowSize = atoi(argv[3]);

				demoScene::loadAnimals(*scene, totalAnimal, rowSize);
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