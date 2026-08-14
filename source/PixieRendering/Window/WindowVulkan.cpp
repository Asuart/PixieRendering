#include "WindowVulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace PixieRenderer {

WindowVulkan::WindowVulkan(const std::string& name, glm::ivec2 resolution)
    : Window(name, resolution, RenderAPI::Vulkan) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(resolution.x, resolution.y, "Vulkan Window", nullptr, nullptr);
}

WindowVulkan::~WindowVulkan() {
	glfwDestroyWindow(m_window);
}

std::vector<const char*> WindowVulkan::GetRequiredExtensions() {
	uint32_t extensionCount = 0;
	const char** rawExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

	std::vector<const char*> extensions(extensionCount);
	for (size_t i = 0; i < extensions.size(); i++) {
		extensions[i] = rawExtensions[i];
	}

	return extensions;
}

void WindowVulkan::CreateSurface(VkInstance vkInstance, VkSurfaceKHR& vkSurface) {
	if (glfwCreateWindowSurface(vkInstance, m_window, nullptr, &vkSurface) != VK_SUCCESS) {
		throw "failed to create window surface!";
	}
}

} // namespace PixieRenderer
