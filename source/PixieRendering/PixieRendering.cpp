#include "PixieRendering.h"

#include "MainWindow/MainWindowOpenGL.h"
#include "MainWindow/MainWindowVulkan.h"

#include "Renderer/OpenGL/RendererOpenGL.h"
#include "Renderer/Vulkan/RendererVulkan.h"

MainWindow* CreateMainWindow(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI) {
    switch (renderAPI) {
        case RenderAPI::OpenGL:
            return new MainWindowOpenGL(name, resolution);
        case RenderAPI::Vulkan:
            return new MainWindowVulkan(name, resolution);
        default: return nullptr;
    }
}

Renderer* CreateRenderer(MainWindow* window) {
    if (!window) {
        return nullptr;
    }
    switch (window->GetRenderAPI()) {
        case RenderAPI::OpenGL:
            return new RendererOpenGL(window);
        default:
            return nullptr;
    }
}
