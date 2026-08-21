#include "Window.h"

#include <iostream>

#include <GLFW/glfw3.h>

#include "../Renderer/IRenderer.h"

namespace PixieRenderer {

Window::Window(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI)
    : m_name(name), m_resolution(resolution), m_renderAPI(renderAPI) {
	if (!glfwInit()) {
		std::cerr << "Failed to Initialize GLFW\n";
		exit(1);
	}
}

Window::~Window() {
	glfwTerminate();
}

void Window::SetRenderer(IRenderer* renderer) {
	m_renderer = renderer;
}

glm::ivec2 Window::GetResolution() const {
	return m_resolution;
}

bool Window::GetShouldClose() const {
	return glfwWindowShouldClose(m_window);
}

RenderAPI Window::GetRenderAPI() const {
	return m_renderAPI;
}

void Window::Close() {
	glfwSetWindowShouldClose(m_window, true);
}

void Window::HandleEvent(const WindowEvent&) {
	// if (event.type == SDL_QUIT) {
	//     Close();
	//     return;
	// }
	// if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
	// event.window.windowID == SDL_GetWindowID(m_window)) { 	Close();
	//     return;
	// }
}

void Window::SwapBuffers() {
	glfwSwapBuffers(m_window);
}

void Window::PollEvents() {
	glfwPollEvents();
}

void Window::OnResize(glm::ivec2 newSize) {
	if (m_resolution == newSize) {
		return;
	}
	m_resolution = newSize;
	if (m_renderer) {
		m_renderer->SetRenderResolution(newSize.x, newSize.y);
	}
}

} // namespace PixieRenderer
