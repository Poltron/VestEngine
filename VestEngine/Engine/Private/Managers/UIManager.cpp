#include "Managers/UIManager.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

// todo: need to isolate GLFW/OpenGL3 specifics

void UIManager::initialize()
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
	ImGui_ImplGlfw_InitForOpenGL(window, true);				// Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void UIManager::startFrame()
{
	// (Your code calls glfwPollEvents())
	// ...
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UIManager::render()
{
	// Rendering
	// (Your code clears your framebuffer, renders your other stuff etc.)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// (Your code calls glfwSwapBuffers() etc.)
}

void UIManager::cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}