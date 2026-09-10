#include "Editor.h"

#include "Engine.h"
#include "Scene.h"
#include "Components/Entity.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"

class VestEditor final : public Editor
{
public:
	bool initialize();
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

	void loadDemoScene()
	{
		engine::getScene()->loadPlaceholderScene();
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
}

bool VestEditor::initialize()
{
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

void VestEditor::shutdown()
{}