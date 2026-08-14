#pragma once
#include "Renderer/IRenderer.h"
#include "Window/Window.h"

namespace PixieRenderer {

Window* CreateWindow(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI);
IRenderer* CreateRenderer(Window* window);

} // namespace PixieRenderer
