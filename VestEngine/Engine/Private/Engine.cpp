#include "Engine.h"

#include "Scene.h"
#include "Helpers/HierarchyHelper.h"
#include "Managers/ResourcesManager.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "Utils/EntityFactory.h"

class VestEngine final : public Engine
{
public:
	bool initialize();
	void launch();
	void shutdown();

private:
	bool isShutdownRequested();
};

namespace engine
{
	VestEngine* g_Engine = nullptr;
	Scene* g_Scene = nullptr;
	ResourcesManager* g_Resources = nullptr;

	Engine* getEngine()
	{
		return g_Engine;
	}

	Scene* getScene()
	{
		return g_Scene;
	}

	ResourcesManager* getResources()
	{
		return g_Resources;
	}

	bool initialize()
	{
		g_Engine = new VestEngine();
		g_Resources = new ResourcesManager();
		g_Scene = new Scene();

		return g_Engine->initialize()
			&& g_Scene->initialize();
	}

	void launch()
	{
		g_Engine->launch();
	}

	void shutdown()
	{
		if (g_Engine)
		{
			g_Engine->shutdown();
		}

		delete g_Scene;
		g_Scene = nullptr;
		delete g_Resources;
		g_Resources = nullptr;
		delete g_Engine;
		g_Engine = nullptr;
	}
}

bool VestEngine::initialize()
{
	uiManager.initialize();

	camera.initialize();
	renderer::getRenderer()->setActiveCamera(&camera);

	return true;
}

void VestEngine::launch()
{
	double lastFrame = platform::getTime();

	while (!isShutdownRequested())
	{
		double currentFrame = platform::getTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		platform::getInputManager().processInput(deltaTime);

		uiManager.startFrame();

		camera.update(deltaTime);

		Scene* scene = engine::getScene();
		ensure(scene);

		scene->hierarchyComponents.drawDebug(50, 50);
		scene->localTransformComponents.drawDebug(300, 50);
		scene->worldTransformComponents.drawDebug(550, 50);

		hierarchySystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents);
		physicsSystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents, scene->rigidbodyComponents, deltaTime);
		transformSystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents);

		renderer::getRenderer()->clear();
		renderer::getRenderer()->render(*engine::getResources(), scene->worldTransformComponents, scene->meshRendererComponents, currentFrame);
		uiManager.render();
		renderer::getRenderer()->swap();
	}
}

void VestEngine::shutdown()
{
	uiManager.cleanup();
}

bool VestEngine::isShutdownRequested()
{
	return platform::getWindowManager().shouldCloseWindow();
}