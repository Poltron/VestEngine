#include "Core/Engine.h"

#include "tracy/Tracy.hpp"

#include "Core/ResourcesManager.h"
#include "Core/Scene.h"
#include "ECS/HierarchyHelper.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "Render/Renderer.h"
#include "UI/UIManager.h"

class VestEngine final : public Engine
{
public:
	bool initialize();
	void launch();
	void shutdown();
};

namespace engine
{
	VestEngine* g_Engine = nullptr;
	Scene* g_Scene = nullptr;
	ResourcesManager* g_Resources = nullptr;
}

//
bool VestEngine::initialize()
{
	setState(EState::INITIALIZING);
	return true;
}

void VestEngine::launch()
{
	setState(EState::RUNNING);

	double lastFrame = platform::getTime();

	while (getState() == EState::RUNNING)
	{
		double currentFrame = platform::getTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		platform::getInputManager().processInput(deltaTime);
		ui::startFrame();

		Scene* scene = engine::getScene();
		ensure(scene);

		render::getRenderer()->updateLightParameters(*scene);

		scene->getCamera().update(deltaTime);
		scene->getCameraDebug().update(deltaTime);

		scene->getHierarchyComponents().drawDebug(50, 50);
		scene->getLocalTransformComponents().drawDebug(300, 50);
		scene->getWorldTransformComponents().drawDebug(550, 50);

		hierarchySystem.update(*scene);
		physicsSystem.update(*scene, deltaTime);
		transformSystem.update(*scene);
		frustumCullingSystem.update(*scene);

		render::getRenderer()->clear();
		render::getRenderer()->render(*scene, currentFrame);
		if (uiUpdateCallback)
		{
			uiUpdateCallback();
		}
		ui::render();
		render::getRenderer()->swap();

		FrameMark;
	}
}

void VestEngine::shutdown()
{
	setState(EState::SHUTDOWN);
}

//
Engine* engine::getEngine() { return g_Engine; }
Scene* engine::getScene() { return g_Scene; }
ResourcesManager* engine::getResources() { return g_Resources; }

bool engine::initialize()
{
	ZoneScoped;

	g_Engine = new VestEngine();
	g_Resources = new ResourcesManager();

	bool bSuccess = g_Engine->initialize();
	if (bSuccess)
	{
		std::cout << "Engine initialization success." << std::endl;
	}
	else
	{
		std::cout << "Engine initialization failed." << std::endl;
	}
	return bSuccess;
}

Scene* engine::createScene()
{
	ZoneScoped;

	g_Scene = new Scene();
	
	if (!g_Scene->initialize())
	{
		g_Scene->shutdown();

		delete g_Scene;
		g_Scene = nullptr;
	}

	return g_Scene;
}

void engine::launch()
{
	g_Engine->launch();
}

void engine::shutdown()
{
	ZoneScoped;

	if (g_Scene)
	{
		g_Scene->shutdown();
	}

	delete g_Scene;
	g_Scene = nullptr;

	delete g_Resources;
	g_Resources = nullptr;

	if (g_Engine)
	{
		g_Engine->shutdown();
	}

	delete g_Engine;
	g_Engine = nullptr;
}