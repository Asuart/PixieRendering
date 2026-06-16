#pragma once
#include <glm/glm.hpp>

#include "PixieRendering/MainWindow/WindowEvent.h"
#include "PixieRendering/RenderAPI.h"

struct GLFWwindow;

class MainWindow {
public:
    MainWindow(const std::string& name, glm::ivec2 resolution);
    virtual ~MainWindow();

    glm::ivec2 GetResolution() const;
    bool GetShouldClose() const;

    virtual void HandleEvent(const WindowEvent& event);
    virtual void SwapBuffers();
    virtual void Close();

protected:
    std::string m_name = "Unnamed Window";
    GLFWwindow* m_window = nullptr;
    glm::ivec2 m_resolution = { 0, 0 };
};
