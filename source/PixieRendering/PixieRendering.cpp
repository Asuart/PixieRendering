#include "PixieRendering.h"

#include "Window/WindowOpenGL.h"
#include "Window/WindowVulkan.h"

#include "Renderer/OpenGL/RendererOpenGL.h"
#include "Renderer/Vulkan/RendererVulkan.h"

namespace PixieRenderer {

Window* CreateMainWindow(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI) {
	switch (renderAPI) {
	case RenderAPI::OpenGL:
		return new WindowOpenGL(name, resolution);
	case RenderAPI::Vulkan:
		return new WindowVulkan(name, resolution);
	default:
		return nullptr;
	}
}

IRenderer* CreateRenderer(Window* window) {
	if (!window) {
		return nullptr;
	}
	switch (window->GetRenderAPI()) {
	case RenderAPI::OpenGL:
		return new RendererOpenGL(window);
	default:
		return new RendererVulkan(window);
	}
}

} // namespace PixieRenderer
