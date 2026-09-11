#include "UI/UIManager.h"

#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

class GLFWOpenGLUIManager final : public UIManager
{
public:
	bool initialize();
	void startFrame();
	void render();
	void shutdown();
};

// todo: need to isolate GLFW/OpenGL3 specifics
namespace ui
{
	GLFWOpenGLUIManager* g_UIManager = nullptr;

	bool initialize()
	{
		return g_UIManager->initialize();
	}

	void startFrame()
	{
		g_UIManager->startFrame();
	}

	void render()
	{
		g_UIManager->render();
	}

	void shutdown()
	{
		g_UIManager->shutdown();
	}

	UIManager* getUIManager()
	{
		return g_UIManager;
	}
}

bool GLFWOpenGLUIManager::initialize()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// IF using Docking Branch

	// Setup Platform/Renderer backends
	GLFWwindow* window = static_cast<GLFWwindow*>(platform::getWindowManager().getWindow());

	// Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true /* install_callbacks */))
	{
		std::cerr << "Error: ImGui GLFW Init failed" << std::endl;
		return false;
	}
	if (!ImGui_ImplOpenGL3_Init())
	{
		std::cerr << "Error: ImGui OpenGL3 Init failed" << std::endl;
		return false;
	}
	return true;
}

void GLFWOpenGLUIManager::startFrame()
{
	// (Your code calls glfwPollEvents())
	// ...
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GLFWOpenGLUIManager::render()
{
	// Rendering
	// (Your code clears your framebuffer, renders your other stuff etc.)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// (Your code calls glfwSwapBuffers() etc.)
}

void GLFWOpenGLUIManager::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}