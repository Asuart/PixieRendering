#pragma once
#include "MainWindow.h"

class MainWindowOpenGL : public MainWindow {
public:
	MainWindowOpenGL(const std::string& name, glm::ivec2 resolution);
	~MainWindowOpenGL();

protected:
	void HandleEvent(const WindowEvent& event) override;
};
