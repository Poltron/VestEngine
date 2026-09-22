#include "Editor/Editor.h"

#include "tracy/Tracy.hpp"

#include "Core/Engine.h"
#include "Core/Scene.h"
#include "ECS/Entity.h"
#include "Platform/InputManager.h"
#include "Platform/Platform.h"
#include "Platform/WindowManager.h"
#include "Render/Renderer.h"

class VestEditor final : public Editor
{
public:
	bool initialize();
	void update();
	void shutdown();
};

namespace editor
{
	VestEditor* g_Editor = nullptr;
	Editor* getEditor() { return g_Editor; }

	bool initialize()
	{
		g_Editor = new VestEditor();
		bool bSuccess = g_Editor->initialize();
		if (bSuccess)
		{
			std::cout << "Editor initialization success." << std::endl << std::endl;
		}
		else
		{
			std::cout << "Editor initialization failed." << std::endl << std::endl;
		}
		return bSuccess;
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
	ZoneScoped;

	getInspector().initialize();
	getEntitySelector().initialize();

	platform::getWindowManager().registerWindowCloseRequestCallback(
		[]()
		{
			engine::getEngine()->setState(Engine::EState::SHUTTING_DOWN);
		});

	platform::getInputManager().registerKeyCallback(input::EKey::ESCAPE
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			engine::getEngine()->setState(Engine::EState::SHUTTING_DOWN);
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
			if (scene->getWorldTransformComponents().size() == 0)
			{
				return;
			}

			size_t index = std::rand() % scene->getWorldTransformComponents().size();
			Entity selectedEntity = scene->getWorldTransformComponents().at(index)->entity;
			if (EntityFuncs::isEntityValid(selectedEntity))
			{
				getEntitySelector().selectEntity(selectedEntity);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::C
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			if (inState != input::EInputState::PRESS)
			{
				return;
			}

			static bool bPingPong = false;
			if (bPingPong)
			{
				render::getRenderer()->setActiveCamera(&engine::getScene()->getCamera());
			}
			else
			{
				render::getRenderer()->setActiveCamera(&engine::getScene()->getCameraDebug());
			}
			bPingPong = !bPingPong;
		});

	engine::getEngine()->registerUIUpdateCallback(
		[this]()
		{
			editor::g_Editor->update();
		});

	return true;
}

void VestEditor::update()
{
	ZoneScoped;

	getInspector().update();
	getEntitySelector().update();
}

void VestEditor::shutdown()
{}