#include "MainWindowOpenGL.h"

#include <iostream>

#include <GLFW/glfw3.h>

MainWindowOpenGL::MainWindowOpenGL(const std::string& name, glm::ivec2 resolution) :
	MainWindow(name, resolution) {

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
}

MainWindowOpenGL::~MainWindowOpenGL() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
}

void MainWindowOpenGL::HandleEvent(const WindowEvent& event) {
    //if (event.type == SDL_WINDOWEVENT) {
    //    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    //        //int32_t newWidth = event.window.data1;
    //        //int32_t newHeight = event.window.data2;
    //        //RenderEngine::ResizeViewport({ newWidth, newHeight });
    //    }
    //}
    MainWindow::HandleEvent(event);
}
