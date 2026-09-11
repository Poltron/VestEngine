#include "Editor/Editor.h"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "ECS/Entity.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

class VestEditor final : public Editor
{
public:
	bool initialize();
	void launch();
	void shutdown();

private:
	Entity selectedEntity;
};

namespace editor
{
	VestEditor* g_Editor = nullptr;
	Editor* getEditor() { return g_Editor; }

	bool initialize()
	{
		g_Editor = new VestEditor();
		return g_Editor->initialize();
	}

	void launch()
	{
		g_Editor->launch();
	}

	void shutdown()
	{
		if (g_Editor)
		{
			g_Editor->shutdown();
		}

		delete g_Editor;
		g_Editor = nullptr;
	}

	void loadDemoScene()
	{
		engine::getScene()->loadPlaceholderScene();
	}
}

bool VestEditor::initialize()
{
	platform::getWindowManager().registerWindowCloseRequestCallback(
		[]()
		{
			engine::getEngine()->setState(Engine::State::SHUTTING_DOWN);
		});

	platform::getInputManager().registerKeyCallback(input::EKey::ESCAPE
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			engine::getEngine()->setState(Engine::State::SHUTTING_DOWN);
		});

	platform::getInputManager().registerKeyCallback(input::EKey::SPACE
		, [this](input::EInputState inState, input::EKeyModifier inMods, double inDeltaTime)
		{
			if (inState != input::EInputState::PRESS)
			{
				return;
			}

			Scene* scene = engine::getScene();
			ensure(scene);

			size_t index = std::rand() % scene->worldTransformComponents.size();
			selectedEntity = scene->worldTransformComponents.at(index)->entity;
		});

	return true;
}

void VestEditor::launch()
{
	engine::launch();
}

void VestEditor::shutdown()
{}