#pragma once
#include "Window.h"

#include <vulkan/vulkan.h>

namespace PixieRenderer {

class RendererVulkan;

class WindowVulkan : public Window {
  public:
	WindowVulkan(const std::string& name, glm::ivec2 resolution);
	~WindowVulkan();

	std::vector<const char*> GetRequiredExtensions();
	void CreateSurface(VkInstance vkInstance, VkSurfaceKHR& vkSurface);

	RendererVulkan* GetRendererVulkan();
};

} // namespace PixieRenderer
