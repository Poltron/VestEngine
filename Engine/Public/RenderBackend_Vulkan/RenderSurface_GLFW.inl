#include "RenderBackend_Vulkan/RenderSurface.h"

#include <iostream>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Platform/Platform.h"
#include "Platform/WindowManager.h"

bool RenderSurface::create(VkInstance inInstance)
{
	WindowManager& windowManager = platform::getWindowManager();
	VkResult result = glfwCreateWindowSurface(inInstance, (GLFWwindow*)windowManager.getWindow(), nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: createWindowSurface failed" << std::endl;
		return false;
	}
	return true;
}

void RenderSurface::destroy(VkInstance inInstance)
{
	vkDestroySurfaceKHR(inInstance, surface, nullptr);
}