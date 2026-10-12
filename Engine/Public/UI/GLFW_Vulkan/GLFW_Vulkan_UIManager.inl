#include "UI/UIManager.h"

#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
//#include "backends/imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

class GLFWVulkanUIManager final : public UIManager
{
public:
	bool initialize();
	void startFrame();
	void render();
	void shutdown();
};

namespace ui
{
	GLFWVulkanUIManager* g_UIManager = nullptr;

	bool initialize()
	{
		ZoneScoped;

		bool bSuccess =  g_UIManager->initialize();
		if (bSuccess)
		{
			std::cout << "UI initialization success." << std::endl;
		}
		else
		{
			std::cout << "UI initialization failed." << std::endl;
		}
		return bSuccess;
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
		ZoneScoped;

		if (g_UIManager)
		{
			g_UIManager->shutdown();
		}

		delete g_UIManager;
		g_UIManager = nullptr;
	}

	UIManager* getUIManager()
	{
		return g_UIManager;
	}
}

bool GLFWVulkanUIManager::initialize()
{
	ZoneScoped;

	//// Setup Dear ImGui context
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// IF using Docking Branch
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//// Setup Platform/Renderer backends
	//GLFWwindow* window = static_cast<GLFWwindow*>(platform::getWindowManager().getWindow());

	//// Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	//if (!ImGui_ImplGlfw_InitForVulkan(window, true /* install_callbacks */))
	//{
	//	std::cerr << "Error: ImGui GLFW Init failed" << std::endl;
	//	return false;
	//}
	//if (!ImGui_ImplVulkan_Init())
	//{
	//	std::cerr << "Error: ImGui Vulkan Init failed" << std::endl;
	//	return false;
	//}
	return true;
}

void GLFWVulkanUIManager::startFrame()
{
	ZoneScoped;

	// (Your code calls glfwPollEvents())
	// ...
	// Start the Dear ImGui frame
	//ImGui_ImplVulkan_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();
}

void GLFWVulkanUIManager::render()
{
	ZoneScoped;
	//TracyGpuZone("GLFWVulkanUIManager::render");

	//// Rendering
	//// (Your code clears your framebuffer, renders your other stuff etc.)
	//ImGui::Render();
	//
	//if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	GLFWwindow* backup_current_context = glfwGetCurrentContext();
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//	glfwMakeContextCurrent(backup_current_context);
	//}

	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
	//// (Your code calls glfwSwapBuffers() etc.)
}

void GLFWVulkanUIManager::shutdown()
{
	ZoneScoped;

	//ImGui_ImplVulkan_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();
}