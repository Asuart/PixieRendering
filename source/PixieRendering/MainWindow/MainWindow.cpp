#include "MainWindow.h"

#include <iostream>

#include <GLFW/glfw3.h>

MainWindow::MainWindow(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI) :
    m_name(name),
    m_resolution(resolution),
    m_renderAPI(renderAPI) {
    if (!glfwInit()) {
        std::cerr << "Failed to Initialize GLFW\n";
        exit(1);
    }
}

MainWindow::~MainWindow() {
    glfwTerminate();
}

glm::ivec2 MainWindow::GetResolution() const {
    return m_resolution;
}

bool MainWindow::GetShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

RenderAPI MainWindow::GetRenderAPI() const {
    return m_renderAPI;
}

void MainWindow::Close() {
    glfwSetWindowShouldClose(m_window, true);
}

void MainWindow::HandleEvent(const WindowEvent&) {
    //if (event.type == SDL_QUIT) {
    //    Close();
    //    return;
    //}
    //if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window)) {
    //	Close();
    //    return;
    //}
}

void MainWindow::SwapBuffers() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}
