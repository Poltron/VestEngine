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

		registerKeyCallback(GLFW_KEY_ESCAPE
			, [this](int state, int mods, double deltaTime)
			{
				glfwSetWindowShouldClose(g_GLFWWindow, true);
			});

		glfwSetInputMode(g_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	std::vector<KeyInput>& getKeyInputs() { return keyInputs; }
	std::vector<MouseInput>& getMouseInputs() { return mouseInputs; }
	std::vector<CursorPosInput>& getCursorPosInputs() { return cursorPosInputs; }
	std::vector<ScrollInput>& getScrollInputs() { return scrollInputs; }


protected:
	virtual void pollEvents() override
	{
		glfwPollEvents();
	}
};

void InputCallbacks::cursorPos_callback(GLFWwindow* window, double xPos, double yPos)
{
	g_GLFWInputManager->getCursorPosInputs().push_back({ xPos, yPos });
}

void InputCallbacks::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	g_GLFWInputManager->getScrollInputs().push_back({ xOffset, yOffset });
}

void InputCallbacks::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	g_GLFWInputManager->getKeyInputs().push_back({ key, action, mods });
	std::cout << key << " / " << scancode << " / " << action << " / " << mods << std::endl;
}

void InputCallbacks::mouseButton_callback(GLFWwindow* window, int button, int action, int mods)
{
	g_GLFWInputManager->getMouseInputs().push_back({ button, action });
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

InputManager& platform::getInputManager()
{
	return *g_GLFWInputManager;
}

WindowManager& platform::getWindowManager()
{
	return *g_GLFWWindowManager;
}