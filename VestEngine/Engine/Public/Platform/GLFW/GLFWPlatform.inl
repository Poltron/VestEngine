#include "Platform/Platform.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Platform/InputManager.h"
#include "Platform/WindowManager.h"

#include "Utils/Ensure.h"

class GLFWPlatform;
class GLFWInputManager;
class GLFWWindowManager;

namespace GLFW {
	GLFWInputManager* g_GLFWInputManager = nullptr;
	GLFWWindowManager* g_GLFWWindowManager = nullptr;
	GLFWwindow* g_GLFWWindow = nullptr;
}
using namespace GLFW;

//
namespace InputCallbacks
{
	void cursorPos_callback(GLFWwindow* window, double xPos, double yPos);
	void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseButton_callback(GLFWwindow* window, int button, int action, int mods);
}

class GLFWInputManager final : public InputManager
{
public:
	virtual void initialize() override
	{
		// input events
		glfwSetKeyCallback(g_GLFWWindow, InputCallbacks::key_callback);
		glfwSetMouseButtonCallback(g_GLFWWindow, InputCallbacks::mouseButton_callback);
		glfwSetScrollCallback(g_GLFWWindow, InputCallbacks::scroll_callback);
		glfwSetCursorPosCallback(g_GLFWWindow, InputCallbacks::cursorPos_callback);

		glfwSetInputMode(g_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	std::vector<KeyInput>& getKeyInputs() { return keyInputs; }
	std::vector<MouseInput>& getMouseInputs() { return mouseInputs; }
	std::vector<CursorPosInput>& getCursorPosInputs() { return cursorPosInputs; }
	std::vector<ScrollInput>& getScrollInputs() { return scrollInputs; }

	input::EKey fromGLFWKey(int inKey)
	{
		switch (inKey)
		{
		case GLFW_KEY_SPACE:
			return input::EKey::SPACE;
		case GLFW_KEY_APOSTROPHE:
			return input::EKey::APOSTROPHE;
		case GLFW_KEY_COMMA:
			return input::EKey::COMMA;
		case GLFW_KEY_MINUS:
			return input::EKey::MINUS;
		case GLFW_KEY_PERIOD:
			return input::EKey::PERIOD;
		case GLFW_KEY_SLASH:
			return input::EKey::SLASH;
		case GLFW_KEY_0:
			return input::EKey::KB_0;
		case GLFW_KEY_1:
			return input::EKey::KB_1;
		case GLFW_KEY_2:
			return input::EKey::KB_2;
		case GLFW_KEY_3:
			return input::EKey::KB_3;
		case GLFW_KEY_4:
			return input::EKey::KB_4;
		case GLFW_KEY_5:
			return input::EKey::KB_5;
		case GLFW_KEY_6:
			return input::EKey::KB_6;
		case GLFW_KEY_7:
			return input::EKey::KB_7;
		case GLFW_KEY_8:
			return input::EKey::KB_8;
		case GLFW_KEY_9:
			return input::EKey::KB_9;
		case GLFW_KEY_SEMICOLON:
			return input::EKey::SEMICOLON;
		case GLFW_KEY_EQUAL:
			return input::EKey::EQUAL;
		case GLFW_KEY_A:
			return input::EKey::A;
		case GLFW_KEY_B:
			return input::EKey::B;
		case GLFW_KEY_C:
			return input::EKey::C;
		case GLFW_KEY_D:
			return input::EKey::D;
		case GLFW_KEY_E:
			return input::EKey::E;
		case GLFW_KEY_F:
			return input::EKey::F;
		case GLFW_KEY_G:
			return input::EKey::G;
		case GLFW_KEY_H:
			return input::EKey::H;
		case GLFW_KEY_I:
			return input::EKey::I;
		case GLFW_KEY_J:
			return input::EKey::J;
		case GLFW_KEY_K:
			return input::EKey::K;
		case GLFW_KEY_L:
			return input::EKey::L;
		case GLFW_KEY_M:
			return input::EKey::M;
		case GLFW_KEY_N:
			return input::EKey::N;
		case GLFW_KEY_O:
			return input::EKey::O;
		case GLFW_KEY_P:
			return input::EKey::P;
		case GLFW_KEY_Q:
			return input::EKey::Q;
		case GLFW_KEY_R:
			return input::EKey::R;
		case GLFW_KEY_S:
			return input::EKey::S;
		case GLFW_KEY_T:
			return input::EKey::T;
		case GLFW_KEY_U:
			return input::EKey::U;
		case GLFW_KEY_V:
			return input::EKey::V;
		case GLFW_KEY_W:
			return input::EKey::W;
		case GLFW_KEY_X:
			return input::EKey::X;
		case GLFW_KEY_Y:
			return input::EKey::Y;
		case GLFW_KEY_Z:
			return input::EKey::Z;
		case GLFW_KEY_LEFT_BRACKET:
			return input::EKey::LEFT_BRACKET;
		case GLFW_KEY_BACKSLASH:
			return input::EKey::BACKSLASH;
		case GLFW_KEY_RIGHT_BRACKET:
			return input::EKey::RIGHT_BRACKET;
		case GLFW_KEY_GRAVE_ACCENT:
			return input::EKey::GRAVE_ACCENT;
		case GLFW_KEY_WORLD_1:
			return input::EKey::WORLD_1;
		case GLFW_KEY_WORLD_2:
			return input::EKey::WORLD_2;
		case GLFW_KEY_ESCAPE:
			return input::EKey::ESCAPE;
		case GLFW_KEY_ENTER:
			return input::EKey::ENTER;
		case GLFW_KEY_TAB:
			return input::EKey::TAB;
		case GLFW_KEY_BACKSPACE:
			return input::EKey::BACKSPACE;
		case GLFW_KEY_INSERT:
			return input::EKey::INSERT;
		case GLFW_KEY_DELETE:
			return input::EKey::DELETE;
		case GLFW_KEY_RIGHT:
			return input::EKey::RIGHT;
		case GLFW_KEY_LEFT:
			return input::EKey::LEFT;
		case GLFW_KEY_DOWN:
			return input::EKey::DOWN;
		case GLFW_KEY_UP:
			return input::EKey::UP;
		case GLFW_KEY_PAGE_UP:
			return input::EKey::PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:
			return input::EKey::PAGE_DOWN;
		case GLFW_KEY_HOME:
			return input::EKey::HOME;
		case GLFW_KEY_END:
			return input::EKey::END;
		case GLFW_KEY_CAPS_LOCK:
			return input::EKey::CAPS_LOCK;
		case GLFW_KEY_SCROLL_LOCK:
			return input::EKey::SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:
			return input::EKey::NUM_LOCK;
		case GLFW_KEY_PRINT_SCREEN:
			return input::EKey::PRINT_SCREEN;
		case GLFW_KEY_PAUSE:
			return input::EKey::PAUSE;
		case GLFW_KEY_F1:
			return input::EKey::F1;
		case GLFW_KEY_F2:
			return input::EKey::F2;
		case GLFW_KEY_F3:
			return input::EKey::F3;
		case GLFW_KEY_F4:
			return input::EKey::F4;
		case GLFW_KEY_F5:
			return input::EKey::F5;
		case GLFW_KEY_F6:
			return input::EKey::F6;
		case GLFW_KEY_F7:
			return input::EKey::F7;
		case GLFW_KEY_F8:
			return input::EKey::F8;
		case GLFW_KEY_F9:
			return input::EKey::F9;
		case GLFW_KEY_F10:
			return input::EKey::F10;
		case GLFW_KEY_F11:
			return input::EKey::F11;
		case GLFW_KEY_F12:
			return input::EKey::F12;
		case GLFW_KEY_F13:
			return input::EKey::F13;
		case GLFW_KEY_F14:
			return input::EKey::F14;
		case GLFW_KEY_F15:
			return input::EKey::F15;
		case GLFW_KEY_F16:
			return input::EKey::F16;
		case GLFW_KEY_F17:
			return input::EKey::F17;
		case GLFW_KEY_F18:
			return input::EKey::F18;
		case GLFW_KEY_F19:
			return input::EKey::F19;
		case GLFW_KEY_F20:
			return input::EKey::F20;
		case GLFW_KEY_F21:
			return input::EKey::F21;
		case GLFW_KEY_F22:
			return input::EKey::F22;
		case GLFW_KEY_F23:
			return input::EKey::F23;
		case GLFW_KEY_F24:
			return input::EKey::F24;
		case GLFW_KEY_F25:
			return input::EKey::F25;
		case GLFW_KEY_KP_0:
			return input::EKey::KP_0;
		case GLFW_KEY_KP_1:
			return input::EKey::KP_1;
		case GLFW_KEY_KP_2:
			return input::EKey::KP_2;
		case GLFW_KEY_KP_3:
			return input::EKey::KP_3;
		case GLFW_KEY_KP_4:
			return input::EKey::KP_4;
		case GLFW_KEY_KP_5:
			return input::EKey::KP_5;
		case GLFW_KEY_KP_6:
			return input::EKey::KP_6;
		case GLFW_KEY_KP_7:
			return input::EKey::KP_7;
		case GLFW_KEY_KP_8:
			return input::EKey::KP_8;
		case GLFW_KEY_KP_9:
			return input::EKey::KP_9;
		case GLFW_KEY_KP_DECIMAL:
			return input::EKey::KP_DECIMAL;
		case GLFW_KEY_KP_DIVIDE:
			return input::EKey::KP_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:
			return input::EKey::KP_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:
			return input::EKey::KP_SUBTRACT;
		case GLFW_KEY_KP_ADD:
			return input::EKey::KP_ADD;
		case GLFW_KEY_KP_ENTER:
			return input::EKey::KP_ENTER;
		case GLFW_KEY_KP_EQUAL:
			return input::EKey::KP_EQUAL;
		case GLFW_KEY_LEFT_SHIFT:
			return input::EKey::LEFT_SHIFT;
		case GLFW_KEY_LEFT_CONTROL:
			return input::EKey::LEFT_CONTROL;
		case GLFW_KEY_LEFT_ALT:
			return input::EKey::LEFT_ALT;
		case GLFW_KEY_LEFT_SUPER:
			return input::EKey::LEFT_SUPER;
		case GLFW_KEY_RIGHT_SHIFT:
			return input::EKey::RIGHT_SHIFT;
		case GLFW_KEY_RIGHT_CONTROL:
			return input::EKey::RIGHT_CONTROL;
		case GLFW_KEY_RIGHT_ALT:
			return input::EKey::RIGHT_ALT;
		case GLFW_KEY_RIGHT_SUPER:
			return input::EKey::RIGHT_SUPER;
		case GLFW_KEY_MENU:
			return input::EKey::MENU;
		};

		return input::EKey::UNKNOWN;
	}

	input::EMouseButton fromGLFWMouseButton(int inButton)
	{
		switch (inButton)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			return input::EMouseButton::LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return input::EMouseButton::RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return input::EMouseButton::MIDDLE;
		case GLFW_MOUSE_BUTTON_4:
			return input::EMouseButton::BUTTON_4;
		case GLFW_MOUSE_BUTTON_5:
			return input::EMouseButton::BUTTON_5;
		case GLFW_MOUSE_BUTTON_6:
			return input::EMouseButton::BUTTON_6;
		case GLFW_MOUSE_BUTTON_7:
			return input::EMouseButton::BUTTON_7;
		case GLFW_MOUSE_BUTTON_8:
			return input::EMouseButton::BUTTON_8;
		}

		return input::EMouseButton::UNKNOWN;
	}

	input::EInputState fromGLFWState(int inState)
	{
		switch (inState)
		{
		case GLFW_PRESS:
			return input::EInputState::PRESS;
		case GLFW_RELEASE:
			return input::EInputState::RELEASE;
		case GLFW_REPEAT:
			return input::EInputState::REPEAT;
			
		}

		return input::EInputState::UNKNOWN;
	}

	input::EKeyModifier fromGLFWKeyModifiers(int inModifiers)
	{
		return input::EKeyModifier::NONE;
	}

protected:
	virtual void pollEvents() override
	{
		glfwPollEvents();
	}
};

void InputCallbacks::cursorPos_callback(GLFWwindow* inWindow, double inX, double inY)
{
	g_GLFWInputManager->getCursorPosInputs().push_back({ inX, inY });
}

void InputCallbacks::scroll_callback(GLFWwindow* inWindow, double inX, double inY)
{
	g_GLFWInputManager->getScrollInputs().push_back({ inX, inY });
}

void InputCallbacks::key_callback(GLFWwindow* inWindow, int inGLFWKey, int inScancode, int inAction, int inMods)
{
	input::EKey key = g_GLFWInputManager->fromGLFWKey(inGLFWKey);
	input::EInputState state = g_GLFWInputManager->fromGLFWState(inAction);
	input::EKeyModifier mods = g_GLFWInputManager->fromGLFWKeyModifiers(inMods);
	g_GLFWInputManager->getKeyInputs().push_back({ key, state, mods });
	std::cout << inGLFWKey << " / " << inScancode << " / " << inAction << " / " << inMods << std::endl;
}

void InputCallbacks::mouseButton_callback(GLFWwindow* inWindow, int inGLFWButton, int inAction, int inMods)
{
	input::EMouseButton button = g_GLFWInputManager->fromGLFWMouseButton(inGLFWButton);
	input::EInputState state = g_GLFWInputManager->fromGLFWState(inAction);
	g_GLFWInputManager->getMouseInputs().push_back({ button, state });
}


//
namespace WindowEvents
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
}

class GLFWWindowManager final : public WindowManager
{
public:
	virtual void* createWindow(int width, int height) override
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		g_GLFWWindow = glfwCreateWindow(width, height, "VestEngine", NULL, NULL);
		if (g_GLFWWindow == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return nullptr;
		}

		window = g_GLFWWindow;

		glfwMakeContextCurrent(g_GLFWWindow);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return nullptr;
		}

		glfwSetFramebufferSizeCallback(g_GLFWWindow, WindowEvents::framebuffer_size_callback);
		
		glViewport(0, 0, width, height);

		return g_GLFWWindow;
	}

	virtual void destroyWindow() override
	{
		glfwTerminate();
	}

	virtual void closeWindow() override
	{
		glfwSetWindowShouldClose(g_GLFWWindow, true);
	}

	virtual bool shouldCloseWindow() const
	{
		return glfwWindowShouldClose(static_cast<GLFWwindow*>(window));
	}

	virtual void swapBuffers() override
	{
		glfwSwapBuffers(static_cast<GLFWwindow*>(window));
	}

	virtual void resize(int inWidth, int inHeight) override
	{
		glViewport(0, 0, inWidth, inHeight);
	}
};

void WindowEvents::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	g_GLFWWindowManager->resize(width, height);
}

//
void platform::initialize()
{
	g_GLFWWindowManager = new GLFWWindowManager();

	const int width = 800;
	const int height = 600;
	void* window = g_GLFWWindowManager->createWindow(width, height);
	if (!window)
	{
		return;
	}

	g_GLFWInputManager = new GLFWInputManager();
	g_GLFWInputManager->initialize();
}

void platform::shutdown()
{
	g_GLFWWindowManager->destroyWindow();
}

double platform::getTime()
{
	return glfwGetTime();
}

InputManager& platform::getInputManager()
{
	return *g_GLFWInputManager;
}

WindowManager& platform::getWindowManager()
{
	return *g_GLFWWindowManager;
}