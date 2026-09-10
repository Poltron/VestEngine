#include "Platform/Platform.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Platform/InputManager.h"
#include "Platform/WindowManager.h"

#include "Utils/Ensure.h"

#define PLATFORM_KEY_SIZE GLFW_KEY_LAST + 1
#define PLATFORM_MOUSE_BUTTON_SIZE GLFW_MOUSE_BUTTON_LAST + 1
#define PLATFORM_INPUT_STATE_SIZE GLFW_REPEAT + 1
#define PLATFORM_KEY_MODIFIERS_SIZE 0 /* todo */

#define PLATFORM_CURSOR_INPUT_MODE_FIRST GLFW_CURSOR_NORMAL
#define PLATFORM_CURSOR_INPUT_MODE_LAST GLFW_CURSOR_CAPTURED
#define PLATFORM_CURSOR_INPUT_MODE_SIZE (PLATFORM_CURSOR_INPUT_MODE_LAST - PLATFORM_CURSOR_INPUT_MODE_FIRST + 1)

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
	void cursorPos_callback(GLFWwindow* inWindow, double inX, double inY);
	void scroll_callback(GLFWwindow* inWindow, double inX, double inY);
	void key_callback(GLFWwindow* inWindow, int inKey, int inScancode, int inAction, int inMods);
	void mouseButton_callback(GLFWwindow* inWindow, int inButton, int inAction, int inMods);
}

class GLFWInputManager final : public InputManager
{
public:
	virtual void initialize() override
	{
		InputManager::initialize();

		fillInputLookupTables();

		glfwSetKeyCallback(g_GLFWWindow, InputCallbacks::key_callback);
		glfwSetMouseButtonCallback(g_GLFWWindow, InputCallbacks::mouseButton_callback);
		glfwSetScrollCallback(g_GLFWWindow, InputCallbacks::scroll_callback);
		glfwSetCursorPosCallback(g_GLFWWindow, InputCallbacks::cursorPos_callback);

		glfwSetInputMode(g_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	std::vector<KeyInput>& getKeyInputs() { return keyInputs; }
	std::vector<MouseInput>& getMouseInputs() { return mouseInputs; }
	std::vector<CursorPosInput>& getCursorPosInputs() { return cursorPosInputs; }
	std::vector<ScrollInput>& getScrollInputs() { return scrollInputs; }

	virtual void setCursorInputMode(input::ECursorInputMode inInputMode) override
	{
		glfwSetInputMode(g_GLFWWindow, GLFW_CURSOR, toGLFWCursorInputMode[(int)inInputMode]);
	}

	virtual input::ECursorInputMode getCursorInputMode() override
	{
		int value = glfwGetInputMode(g_GLFWWindow, GLFW_CURSOR);
		return fromGLFWCursorInputMode[value - PLATFORM_CURSOR_INPUT_MODE_FIRST];
	}

	virtual bool isKeyPressed(input::EKey inKey) override
	{
		ensure(inKey != input::EKey::UNKNOWN);

		int glfwKey = toGLFWKey[static_cast<int>(inKey)];
		int state = glfwGetKey(g_GLFWWindow, glfwKey);
		return fromGLFWInputState[state] == input::EInputState::PRESS;
	}

	virtual bool isButtonPressed(input::EMouseButton inButton) override
	{
		ensure(inButton != input::EMouseButton::UNKNOWN);

		int glfwButton = toGLFWMouseButton[static_cast<int>(inButton)];
		int state = glfwGetMouseButton(g_GLFWWindow, glfwButton);
		return fromGLFWInputState[state] == input::EInputState::PRESS;
	}

	input::EKey fromGLFWKey[PLATFORM_KEY_SIZE];
	int toGLFWKey[(int)input::EKey::ENUM_SIZE];

	input::EMouseButton fromGLFWMouseButton[PLATFORM_MOUSE_BUTTON_SIZE];
	int toGLFWMouseButton[(int)input::EMouseButton::ENUM_SIZE];

	input::EInputState fromGLFWInputState[PLATFORM_INPUT_STATE_SIZE];
	int toGLFWInputState[(int)input::EInputState::ENUM_SIZE];

	input::ECursorInputMode fromGLFWCursorInputMode[PLATFORM_CURSOR_INPUT_MODE_SIZE];
	int toGLFWCursorInputMode[(int)input::ECursorInputMode::ENUM_SIZE];

protected:
	virtual void pollEvents() override
	{
		glfwPollEvents();
	}

private:
	// note: would like some const arrays but GLFW values are not sequential,
	// would like designated initializers but they aren't available until C++20
	void fillInputLookupTables()
	{
		fromGLFWKey[GLFW_KEY_SPACE] = input::EKey::SPACE;
		fromGLFWKey[GLFW_KEY_APOSTROPHE] = input::EKey::APOSTROPHE;
		fromGLFWKey[GLFW_KEY_COMMA]	= input::EKey::COMMA;
		fromGLFWKey[GLFW_KEY_MINUS] = input::EKey::MINUS;
		fromGLFWKey[GLFW_KEY_PERIOD] = input::EKey::PERIOD;
		fromGLFWKey[GLFW_KEY_SLASH] = input::EKey::SLASH;
		fromGLFWKey[GLFW_KEY_0] = input::EKey::KB_0;
		fromGLFWKey[GLFW_KEY_1] = input::EKey::KB_1;
		fromGLFWKey[GLFW_KEY_2] = input::EKey::KB_2;
		fromGLFWKey[GLFW_KEY_3] = input::EKey::KB_3;
		fromGLFWKey[GLFW_KEY_4] = input::EKey::KB_4;
		fromGLFWKey[GLFW_KEY_5] = input::EKey::KB_5;
		fromGLFWKey[GLFW_KEY_6] = input::EKey::KB_6;
		fromGLFWKey[GLFW_KEY_7] = input::EKey::KB_7;
		fromGLFWKey[GLFW_KEY_8] = input::EKey::KB_8;
		fromGLFWKey[GLFW_KEY_9] = input::EKey::KB_9;
		fromGLFWKey[GLFW_KEY_SEMICOLON] = input::EKey::SEMICOLON;
		fromGLFWKey[GLFW_KEY_EQUAL] = input::EKey::EQUAL;
		fromGLFWKey[GLFW_KEY_A] = input::EKey::A;
		fromGLFWKey[GLFW_KEY_B] = input::EKey::B;
		fromGLFWKey[GLFW_KEY_C] = input::EKey::C;
		fromGLFWKey[GLFW_KEY_D] = input::EKey::D;
		fromGLFWKey[GLFW_KEY_E] = input::EKey::E;
		fromGLFWKey[GLFW_KEY_F] = input::EKey::F;
		fromGLFWKey[GLFW_KEY_G] = input::EKey::G;
		fromGLFWKey[GLFW_KEY_H] = input::EKey::H;
		fromGLFWKey[GLFW_KEY_I] = input::EKey::I;
		fromGLFWKey[GLFW_KEY_J] = input::EKey::J;
		fromGLFWKey[GLFW_KEY_K] = input::EKey::K;
		fromGLFWKey[GLFW_KEY_L] = input::EKey::L;
		fromGLFWKey[GLFW_KEY_M] = input::EKey::M;
		fromGLFWKey[GLFW_KEY_N] = input::EKey::N;
		fromGLFWKey[GLFW_KEY_O] = input::EKey::O;
		fromGLFWKey[GLFW_KEY_P] = input::EKey::P;
		fromGLFWKey[GLFW_KEY_Q] = input::EKey::Q;
		fromGLFWKey[GLFW_KEY_R] = input::EKey::R;
		fromGLFWKey[GLFW_KEY_S] = input::EKey::S;
		fromGLFWKey[GLFW_KEY_T] = input::EKey::T;
		fromGLFWKey[GLFW_KEY_U] = input::EKey::U;
		fromGLFWKey[GLFW_KEY_V] = input::EKey::V;
		fromGLFWKey[GLFW_KEY_W] = input::EKey::W;
		fromGLFWKey[GLFW_KEY_X] = input::EKey::X;
		fromGLFWKey[GLFW_KEY_Y] = input::EKey::Y;
		fromGLFWKey[GLFW_KEY_Z] = input::EKey::Z;
		fromGLFWKey[GLFW_KEY_LEFT_BRACKET] = input::EKey::LEFT_BRACKET;
		fromGLFWKey[GLFW_KEY_BACKSLASH] = input::EKey::BACKSLASH;
		fromGLFWKey[GLFW_KEY_RIGHT_BRACKET] = input::EKey::RIGHT_BRACKET;
		fromGLFWKey[GLFW_KEY_GRAVE_ACCENT] = input::EKey::GRAVE_ACCENT;
		fromGLFWKey[GLFW_KEY_WORLD_1] = input::EKey::WORLD_1;
		fromGLFWKey[GLFW_KEY_WORLD_2] = input::EKey::WORLD_2;
		fromGLFWKey[GLFW_KEY_ESCAPE] = input::EKey::ESCAPE;
		fromGLFWKey[GLFW_KEY_ENTER] = input::EKey::ENTER;
		fromGLFWKey[GLFW_KEY_TAB] = input::EKey::TAB;
		fromGLFWKey[GLFW_KEY_BACKSPACE] = input::EKey::BACKSPACE;
		fromGLFWKey[GLFW_KEY_INSERT] = input::EKey::INSERT;
		fromGLFWKey[GLFW_KEY_DELETE] = input::EKey::DELETE;
		fromGLFWKey[GLFW_KEY_RIGHT] = input::EKey::RIGHT;
		fromGLFWKey[GLFW_KEY_LEFT] = input::EKey::LEFT;
		fromGLFWKey[GLFW_KEY_DOWN] = input::EKey::DOWN;
		fromGLFWKey[GLFW_KEY_UP] = input::EKey::UP;
		fromGLFWKey[GLFW_KEY_PAGE_UP] = input::EKey::PAGE_UP;
		fromGLFWKey[GLFW_KEY_PAGE_DOWN] = input::EKey::PAGE_DOWN;
		fromGLFWKey[GLFW_KEY_HOME] = input::EKey::HOME;
		fromGLFWKey[GLFW_KEY_END] = input::EKey::END;
		fromGLFWKey[GLFW_KEY_CAPS_LOCK] = input::EKey::CAPS_LOCK;
		fromGLFWKey[GLFW_KEY_SCROLL_LOCK] = input::EKey::SCROLL_LOCK;
		fromGLFWKey[GLFW_KEY_NUM_LOCK] = input::EKey::NUM_LOCK;
		fromGLFWKey[GLFW_KEY_PRINT_SCREEN] = input::EKey::PRINT_SCREEN;
		fromGLFWKey[GLFW_KEY_PAUSE] = input::EKey::PAUSE;
		fromGLFWKey[GLFW_KEY_F1] = input::EKey::F1;
		fromGLFWKey[GLFW_KEY_F2] = input::EKey::F2;
		fromGLFWKey[GLFW_KEY_F3] = input::EKey::F3;
		fromGLFWKey[GLFW_KEY_F4] = input::EKey::F4;
		fromGLFWKey[GLFW_KEY_F5] = input::EKey::F5;
		fromGLFWKey[GLFW_KEY_F6] = input::EKey::F6;
		fromGLFWKey[GLFW_KEY_F7] = input::EKey::F7;
		fromGLFWKey[GLFW_KEY_F8] = input::EKey::F8;
		fromGLFWKey[GLFW_KEY_F9] = input::EKey::F9;
		fromGLFWKey[GLFW_KEY_F10] = input::EKey::F10;
		fromGLFWKey[GLFW_KEY_F11] = input::EKey::F11;
		fromGLFWKey[GLFW_KEY_F12] = input::EKey::F12;
		fromGLFWKey[GLFW_KEY_F13] = input::EKey::F13;
		fromGLFWKey[GLFW_KEY_F14] = input::EKey::F14;
		fromGLFWKey[GLFW_KEY_F15] = input::EKey::F15;
		fromGLFWKey[GLFW_KEY_F16] = input::EKey::F16;
		fromGLFWKey[GLFW_KEY_F17] = input::EKey::F17;
		fromGLFWKey[GLFW_KEY_F18] = input::EKey::F18;
		fromGLFWKey[GLFW_KEY_F19] = input::EKey::F19;
		fromGLFWKey[GLFW_KEY_F20] = input::EKey::F20;
		fromGLFWKey[GLFW_KEY_F21] = input::EKey::F21;
		fromGLFWKey[GLFW_KEY_F22] = input::EKey::F22;
		fromGLFWKey[GLFW_KEY_F23] = input::EKey::F23;
		fromGLFWKey[GLFW_KEY_F24] = input::EKey::F24;
		fromGLFWKey[GLFW_KEY_F25] = input::EKey::F25;
		fromGLFWKey[GLFW_KEY_KP_0] = input::EKey::KP_0;
		fromGLFWKey[GLFW_KEY_KP_1] = input::EKey::KP_1;
		fromGLFWKey[GLFW_KEY_KP_2] = input::EKey::KP_2;
		fromGLFWKey[GLFW_KEY_KP_3] = input::EKey::KP_3;
		fromGLFWKey[GLFW_KEY_KP_4] = input::EKey::KP_4;
		fromGLFWKey[GLFW_KEY_KP_5] = input::EKey::KP_5;
		fromGLFWKey[GLFW_KEY_KP_6] = input::EKey::KP_6;
		fromGLFWKey[GLFW_KEY_KP_7] = input::EKey::KP_7;
		fromGLFWKey[GLFW_KEY_KP_8] = input::EKey::KP_8;
		fromGLFWKey[GLFW_KEY_KP_9] = input::EKey::KP_9;
		fromGLFWKey[GLFW_KEY_KP_DECIMAL] = input::EKey::KP_DECIMAL;
		fromGLFWKey[GLFW_KEY_KP_DIVIDE] = input::EKey::KP_DIVIDE;
		fromGLFWKey[GLFW_KEY_KP_MULTIPLY] = input::EKey::KP_MULTIPLY;
		fromGLFWKey[GLFW_KEY_KP_SUBTRACT] = input::EKey::KP_SUBTRACT;
		fromGLFWKey[GLFW_KEY_KP_ADD] = input::EKey::KP_ADD;
		fromGLFWKey[GLFW_KEY_KP_ENTER] = input::EKey::KP_ENTER;
		fromGLFWKey[GLFW_KEY_KP_EQUAL] = input::EKey::KP_EQUAL;
		fromGLFWKey[GLFW_KEY_LEFT_SHIFT] = input::EKey::LEFT_SHIFT;
		fromGLFWKey[GLFW_KEY_LEFT_CONTROL] = input::EKey::LEFT_CONTROL;
		fromGLFWKey[GLFW_KEY_LEFT_ALT] = input::EKey::LEFT_ALT;
		fromGLFWKey[GLFW_KEY_LEFT_SUPER] = input::EKey::LEFT_SUPER;
		fromGLFWKey[GLFW_KEY_RIGHT_SHIFT] = input::EKey::RIGHT_SHIFT;
		fromGLFWKey[GLFW_KEY_RIGHT_CONTROL] = input::EKey::RIGHT_CONTROL;
		fromGLFWKey[GLFW_KEY_RIGHT_ALT] = input::EKey::RIGHT_ALT;
		fromGLFWKey[GLFW_KEY_RIGHT_SUPER] = input::EKey::RIGHT_SUPER;
		fromGLFWKey[GLFW_KEY_MENU] = input::EKey::MENU;

		toGLFWKey[(int)input::EKey::SPACE] = GLFW_KEY_SPACE;
		toGLFWKey[(int)input::EKey::APOSTROPHE] = GLFW_KEY_APOSTROPHE;
		toGLFWKey[(int)input::EKey::COMMA] = GLFW_KEY_COMMA;
		toGLFWKey[(int)input::EKey::MINUS] = GLFW_KEY_MINUS;
		toGLFWKey[(int)input::EKey::PERIOD] = GLFW_KEY_PERIOD;
		toGLFWKey[(int)input::EKey::SLASH] = GLFW_KEY_SLASH;
		toGLFWKey[(int)input::EKey::KB_0] = GLFW_KEY_0;
		toGLFWKey[(int)input::EKey::KB_1] = GLFW_KEY_1;
		toGLFWKey[(int)input::EKey::KB_2] = GLFW_KEY_2;
		toGLFWKey[(int)input::EKey::KB_3] = GLFW_KEY_3;
		toGLFWKey[(int)input::EKey::KB_4] = GLFW_KEY_4;
		toGLFWKey[(int)input::EKey::KB_5] = GLFW_KEY_5;
		toGLFWKey[(int)input::EKey::KB_6] = GLFW_KEY_6;
		toGLFWKey[(int)input::EKey::KB_7] = GLFW_KEY_7;
		toGLFWKey[(int)input::EKey::KB_8] = GLFW_KEY_8;
		toGLFWKey[(int)input::EKey::KB_9] = GLFW_KEY_9;
		toGLFWKey[(int)input::EKey::SEMICOLON] = GLFW_KEY_SEMICOLON;
		toGLFWKey[(int)input::EKey::EQUAL] = GLFW_KEY_EQUAL;
		toGLFWKey[(int)input::EKey::A] = GLFW_KEY_A;
		toGLFWKey[(int)input::EKey::B] = GLFW_KEY_B;
		toGLFWKey[(int)input::EKey::C] = GLFW_KEY_C;
		toGLFWKey[(int)input::EKey::D] = GLFW_KEY_D;
		toGLFWKey[(int)input::EKey::E] = GLFW_KEY_E;
		toGLFWKey[(int)input::EKey::F] = GLFW_KEY_F;
		toGLFWKey[(int)input::EKey::G] = GLFW_KEY_G;
		toGLFWKey[(int)input::EKey::H] = GLFW_KEY_H;
		toGLFWKey[(int)input::EKey::I] = GLFW_KEY_I;
		toGLFWKey[(int)input::EKey::J] = GLFW_KEY_J;
		toGLFWKey[(int)input::EKey::K] = GLFW_KEY_K;
		toGLFWKey[(int)input::EKey::L] = GLFW_KEY_L;
		toGLFWKey[(int)input::EKey::M] = GLFW_KEY_M;
		toGLFWKey[(int)input::EKey::N] = GLFW_KEY_N;
		toGLFWKey[(int)input::EKey::O] = GLFW_KEY_O;
		toGLFWKey[(int)input::EKey::P] = GLFW_KEY_P;
		toGLFWKey[(int)input::EKey::Q] = GLFW_KEY_Q;
		toGLFWKey[(int)input::EKey::R] = GLFW_KEY_R;
		toGLFWKey[(int)input::EKey::S] = GLFW_KEY_S;
		toGLFWKey[(int)input::EKey::T] = GLFW_KEY_T;
		toGLFWKey[(int)input::EKey::U] = GLFW_KEY_U;
		toGLFWKey[(int)input::EKey::V] = GLFW_KEY_V;
		toGLFWKey[(int)input::EKey::W] = GLFW_KEY_W;
		toGLFWKey[(int)input::EKey::X] = GLFW_KEY_X;
		toGLFWKey[(int)input::EKey::Y] = GLFW_KEY_Y;
		toGLFWKey[(int)input::EKey::Z] = GLFW_KEY_Z;
		toGLFWKey[(int)input::EKey::LEFT_BRACKET] = GLFW_KEY_LEFT_BRACKET;
		toGLFWKey[(int)input::EKey::BACKSLASH] = GLFW_KEY_BACKSLASH;
		toGLFWKey[(int)input::EKey::RIGHT_BRACKET] = GLFW_KEY_RIGHT_BRACKET;
		toGLFWKey[(int)input::EKey::GRAVE_ACCENT] = GLFW_KEY_GRAVE_ACCENT;
		toGLFWKey[(int)input::EKey::WORLD_1] = GLFW_KEY_WORLD_1;
		toGLFWKey[(int)input::EKey::WORLD_2] = GLFW_KEY_WORLD_2;
		toGLFWKey[(int)input::EKey::ESCAPE] = GLFW_KEY_ESCAPE;
		toGLFWKey[(int)input::EKey::ENTER] = GLFW_KEY_ENTER;
		toGLFWKey[(int)input::EKey::TAB] = GLFW_KEY_TAB;
		toGLFWKey[(int)input::EKey::BACKSPACE] = GLFW_KEY_BACKSPACE;
		toGLFWKey[(int)input::EKey::INSERT] = GLFW_KEY_INSERT;
		toGLFWKey[(int)input::EKey::DELETE] = GLFW_KEY_DELETE;
		toGLFWKey[(int)input::EKey::RIGHT] = GLFW_KEY_RIGHT;
		toGLFWKey[(int)input::EKey::LEFT] = GLFW_KEY_LEFT;
		toGLFWKey[(int)input::EKey::DOWN] = GLFW_KEY_DOWN;
		toGLFWKey[(int)input::EKey::UP] = GLFW_KEY_UP;
		toGLFWKey[(int)input::EKey::PAGE_UP] = GLFW_KEY_PAGE_UP;
		toGLFWKey[(int)input::EKey::PAGE_DOWN] = GLFW_KEY_PAGE_DOWN;
		toGLFWKey[(int)input::EKey::HOME] = GLFW_KEY_HOME;
		toGLFWKey[(int)input::EKey::END] = GLFW_KEY_END;
		toGLFWKey[(int)input::EKey::CAPS_LOCK] = GLFW_KEY_CAPS_LOCK;
		toGLFWKey[(int)input::EKey::SCROLL_LOCK] = GLFW_KEY_SCROLL_LOCK;
		toGLFWKey[(int)input::EKey::NUM_LOCK] = GLFW_KEY_NUM_LOCK;
		toGLFWKey[(int)input::EKey::PRINT_SCREEN] = GLFW_KEY_PRINT_SCREEN;
		toGLFWKey[(int)input::EKey::PAUSE] = GLFW_KEY_PAUSE;
		toGLFWKey[(int)input::EKey::F1] = GLFW_KEY_F1;
		toGLFWKey[(int)input::EKey::F2] = GLFW_KEY_F2;
		toGLFWKey[(int)input::EKey::F3] = GLFW_KEY_F3;
		toGLFWKey[(int)input::EKey::F4] = GLFW_KEY_F4;
		toGLFWKey[(int)input::EKey::F5] = GLFW_KEY_F5;
		toGLFWKey[(int)input::EKey::F6] = GLFW_KEY_F6;
		toGLFWKey[(int)input::EKey::F7] = GLFW_KEY_F7;
		toGLFWKey[(int)input::EKey::F8] = GLFW_KEY_F8;
		toGLFWKey[(int)input::EKey::F9] = GLFW_KEY_F9;
		toGLFWKey[(int)input::EKey::F10] = GLFW_KEY_F10;
		toGLFWKey[(int)input::EKey::F11] = GLFW_KEY_F11;
		toGLFWKey[(int)input::EKey::F12] = GLFW_KEY_F12;
		toGLFWKey[(int)input::EKey::F13] = GLFW_KEY_F13;
		toGLFWKey[(int)input::EKey::F14] = GLFW_KEY_F14;
		toGLFWKey[(int)input::EKey::F15] = GLFW_KEY_F15;
		toGLFWKey[(int)input::EKey::F16] = GLFW_KEY_F16;
		toGLFWKey[(int)input::EKey::F17] = GLFW_KEY_F17;
		toGLFWKey[(int)input::EKey::F18] = GLFW_KEY_F18;
		toGLFWKey[(int)input::EKey::F19] = GLFW_KEY_F19;
		toGLFWKey[(int)input::EKey::F20] = GLFW_KEY_F20;
		toGLFWKey[(int)input::EKey::F21] = GLFW_KEY_F21;
		toGLFWKey[(int)input::EKey::F22] = GLFW_KEY_F22;
		toGLFWKey[(int)input::EKey::F23] = GLFW_KEY_F23;
		toGLFWKey[(int)input::EKey::F24] = GLFW_KEY_F24;
		toGLFWKey[(int)input::EKey::F25] = GLFW_KEY_F25;
		toGLFWKey[(int)input::EKey::KP_0] = GLFW_KEY_KP_0;
		toGLFWKey[(int)input::EKey::KP_1] = GLFW_KEY_KP_1;
		toGLFWKey[(int)input::EKey::KP_2] = GLFW_KEY_KP_2;
		toGLFWKey[(int)input::EKey::KP_3] = GLFW_KEY_KP_3;
		toGLFWKey[(int)input::EKey::KP_4] = GLFW_KEY_KP_4;
		toGLFWKey[(int)input::EKey::KP_5] = GLFW_KEY_KP_5;
		toGLFWKey[(int)input::EKey::KP_6] = GLFW_KEY_KP_6;
		toGLFWKey[(int)input::EKey::KP_7] = GLFW_KEY_KP_7;
		toGLFWKey[(int)input::EKey::KP_8] = GLFW_KEY_KP_8;
		toGLFWKey[(int)input::EKey::KP_9] = GLFW_KEY_KP_9;
		toGLFWKey[(int)input::EKey::KP_DECIMAL] = GLFW_KEY_KP_DECIMAL;
		toGLFWKey[(int)input::EKey::KP_DIVIDE] = GLFW_KEY_KP_DIVIDE;
		toGLFWKey[(int)input::EKey::KP_MULTIPLY] = GLFW_KEY_KP_MULTIPLY;
		toGLFWKey[(int)input::EKey::KP_SUBTRACT] = GLFW_KEY_KP_SUBTRACT;
		toGLFWKey[(int)input::EKey::KP_ADD] = GLFW_KEY_KP_ADD;
		toGLFWKey[(int)input::EKey::KP_ENTER] = GLFW_KEY_KP_ENTER;
		toGLFWKey[(int)input::EKey::KP_EQUAL] = GLFW_KEY_KP_EQUAL;
		toGLFWKey[(int)input::EKey::LEFT_SHIFT] = GLFW_KEY_LEFT_SHIFT;
		toGLFWKey[(int)input::EKey::LEFT_CONTROL] = GLFW_KEY_LEFT_CONTROL;
		toGLFWKey[(int)input::EKey::LEFT_ALT] = GLFW_KEY_LEFT_ALT;
		toGLFWKey[(int)input::EKey::LEFT_SUPER] = GLFW_KEY_LEFT_SUPER;
		toGLFWKey[(int)input::EKey::RIGHT_SHIFT] = GLFW_KEY_RIGHT_SHIFT;
		toGLFWKey[(int)input::EKey::RIGHT_CONTROL] = GLFW_KEY_RIGHT_CONTROL;
		toGLFWKey[(int)input::EKey::RIGHT_ALT] = GLFW_KEY_RIGHT_ALT;
		toGLFWKey[(int)input::EKey::RIGHT_SUPER] = GLFW_KEY_RIGHT_SUPER;
		toGLFWKey[(int)input::EKey::MENU] = GLFW_KEY_MENU;

		//
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_LEFT] = input::EMouseButton::LEFT;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_RIGHT] = input::EMouseButton::RIGHT;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_MIDDLE] = input::EMouseButton::MIDDLE;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_4] = input::EMouseButton::BUTTON_4;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_5] = input::EMouseButton::BUTTON_5;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_6] = input::EMouseButton::BUTTON_6;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_7] = input::EMouseButton::BUTTON_7;
		fromGLFWMouseButton[GLFW_MOUSE_BUTTON_8] = input::EMouseButton::BUTTON_8;

		toGLFWMouseButton[(int)input::EMouseButton::LEFT] = GLFW_MOUSE_BUTTON_LEFT;
		toGLFWMouseButton[(int)input::EMouseButton::RIGHT] = GLFW_MOUSE_BUTTON_RIGHT;
		toGLFWMouseButton[(int)input::EMouseButton::MIDDLE] = GLFW_MOUSE_BUTTON_MIDDLE;
		toGLFWMouseButton[(int)input::EMouseButton::BUTTON_4] = GLFW_MOUSE_BUTTON_4;
		toGLFWMouseButton[(int)input::EMouseButton::BUTTON_5] = GLFW_MOUSE_BUTTON_5;
		toGLFWMouseButton[(int)input::EMouseButton::BUTTON_6] = GLFW_MOUSE_BUTTON_6;
		toGLFWMouseButton[(int)input::EMouseButton::BUTTON_7] = GLFW_MOUSE_BUTTON_7;
		toGLFWMouseButton[(int)input::EMouseButton::BUTTON_8] = GLFW_MOUSE_BUTTON_8;

		//
		fromGLFWInputState[GLFW_RELEASE] = input::EInputState::RELEASE;
		fromGLFWInputState[GLFW_PRESS] = input::EInputState::PRESS;
		fromGLFWInputState[GLFW_REPEAT] = input::EInputState::REPEAT;

		toGLFWInputState[(int)input::EInputState::RELEASE] = GLFW_RELEASE;
		toGLFWInputState[(int)input::EInputState::PRESS] = GLFW_PRESS;
		toGLFWInputState[(int)input::EInputState::REPEAT] = GLFW_REPEAT;

		//
		fromGLFWCursorInputMode[GLFW_CURSOR_NORMAL - PLATFORM_CURSOR_INPUT_MODE_FIRST] = input::ECursorInputMode::NORMAL;
		fromGLFWCursorInputMode[GLFW_CURSOR_HIDDEN - PLATFORM_CURSOR_INPUT_MODE_FIRST] = input::ECursorInputMode::HIDDEN;
		fromGLFWCursorInputMode[GLFW_CURSOR_DISABLED - PLATFORM_CURSOR_INPUT_MODE_FIRST] = input::ECursorInputMode::DISABLED;
		fromGLFWCursorInputMode[GLFW_CURSOR_CAPTURED - PLATFORM_CURSOR_INPUT_MODE_FIRST] = input::ECursorInputMode::CAPTURED;

		toGLFWCursorInputMode[(int)input::ECursorInputMode::NORMAL] = GLFW_CURSOR_NORMAL;
		toGLFWCursorInputMode[(int)input::ECursorInputMode::HIDDEN] = GLFW_CURSOR_HIDDEN;
		toGLFWCursorInputMode[(int)input::ECursorInputMode::DISABLED] = GLFW_CURSOR_DISABLED;
		toGLFWCursorInputMode[(int)input::ECursorInputMode::CAPTURED] = GLFW_CURSOR_CAPTURED;
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
	input::EKey key = g_GLFWInputManager->fromGLFWKey[inGLFWKey];
	input::EInputState state = g_GLFWInputManager->fromGLFWInputState[inAction];
	input::EKeyModifier mods = input::EKeyModifier::NONE; // todo
	g_GLFWInputManager->getKeyInputs().push_back({ key, state, mods });
	std::cout << "Key: " << inGLFWKey << " / " << inAction << " / " << inMods << std::endl;
}

void InputCallbacks::mouseButton_callback(GLFWwindow* inWindow, int inGLFWButton, int inAction, int inMods)
{
	input::EMouseButton button = g_GLFWInputManager->fromGLFWMouseButton[inGLFWButton];
	input::EInputState state = g_GLFWInputManager->fromGLFWInputState[inAction];
	g_GLFWInputManager->getMouseInputs().push_back({ button, state });
	std::cout << "Button: " << inGLFWButton << " / " << inAction << " / " << inMods << std::endl;
}


//
namespace WindowEvents
{
	void framebuffer_size_callback(GLFWwindow* inWindow, int inWidth, int inHeight);
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

void WindowEvents::framebuffer_size_callback(GLFWwindow* inWindow, int inWidth, int inHeight)
{
	g_GLFWWindowManager->resize(inWidth, inHeight);
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