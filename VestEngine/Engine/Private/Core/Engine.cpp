#include "Core/Engine.h"

#include "Core/ResourcesManager.h"
#include "Core/Scene.h"
#include "ECS/EntityFactory.h"
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
	setState(State::INITIALIZING);

	// could probably go to editor ?
	camera.initialize();
	render::getRenderer()->setActiveCamera(&camera);

	return true;
}

void VestEngine::launch()
{
	setState(State::RUNNING);

	double lastFrame = platform::getTime();

	while (getState() == State::RUNNING)
	{
		double currentFrame = platform::getTime();
		double deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		platform::getInputManager().processInput(deltaTime);

		ui::startFrame();

		camera.update(deltaTime);

		Scene* scene = engine::getScene();
		ensure(scene);

		scene->hierarchyComponents.drawDebug(50, 50);
		scene->localTransformComponents.drawDebug(300, 50);
		scene->worldTransformComponents.drawDebug(550, 50);

		hierarchySystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents);
		physicsSystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents, scene->rigidbodyComponents, deltaTime);
		transformSystem.update(scene->localTransformComponents, scene->worldTransformComponents, scene->hierarchyComponents);

		render::getRenderer()->clear();
		render::getRenderer()->render(*engine::getResources(), scene->worldTransformComponents, scene->meshRendererComponents, currentFrame);
		if (uiUpdateCallback)
		{
			uiUpdateCallback();
		}
		ui::render();
		render::getRenderer()->swap();
	}
}

void VestEngine::shutdown()
{
	setState(State::SHUTDOWN);
}

//
Engine* engine::getEngine() { return g_Engine; }
Scene* engine::getScene() { return g_Scene; }
ResourcesManager* engine::getResources() { return g_Resources; }

bool engine::initialize()
{
	g_Engine = new VestEngine();
	g_Resources = new ResourcesManager();
	g_Scene = new Scene();

	return g_Engine->initialize() && g_Scene->initialize();
}

void engine::launch()
{
	g_Engine->launch();
}

void engine::shutdown()
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