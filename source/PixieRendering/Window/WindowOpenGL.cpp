#include "WindowOpenGL.h"

#include <iostream>

#include <GLFW/glfw3.h>

namespace PixieRenderer {

WindowOpenGL::WindowOpenGL(const std::string& name, glm::ivec2 resolution)
    : Window(name, resolution, RenderAPI::OpenGL) {

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(resolution.x, resolution.y, name.c_str(), NULL, NULL);
	if (!m_window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		exit(1);
	}

	glfwMakeContextCurrent(m_window);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (wnd) {
			wnd->OnResize(glm::ivec2(width, height));
		}
	});
}

WindowOpenGL::~WindowOpenGL() {
	if (m_window) {
		glfwDestroyWindow(m_window);
	}
}

void WindowOpenGL::HandleEvent(const WindowEvent& event) {
	Window::HandleEvent(event);
}

} // namespace PixieRenderer
