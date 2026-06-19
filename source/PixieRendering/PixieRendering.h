#pragma once
#include "Renderer/Renderer.h"
#include "MainWindow/MainWindow.h"

MainWindow* CreateMainWindow(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI);
Renderer* CreateRenderer(MainWindow* window);
