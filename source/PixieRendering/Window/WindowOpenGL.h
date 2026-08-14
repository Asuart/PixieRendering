#pragma once
#include "Window.h"

namespace PixieRenderer {

class WindowOpenGL : public Window {
  public:
	WindowOpenGL(const std::string& name, glm::ivec2 resolution);
	~WindowOpenGL();

  protected:
	void HandleEvent(const WindowEvent& event) override;
};

} // namespace PixieRenderer
