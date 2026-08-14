#pragma once
#include <glm/glm.hpp>

#include "PixieRendering/Window/WindowEvent.h"
#include "PixieRendering/RenderAPI.h"

struct GLFWwindow;

namespace PixieRenderer {

class Window {
  public:
	Window(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI);
	virtual ~Window();

	glm::ivec2 GetResolution() const;
	bool GetShouldClose() const;
	RenderAPI GetRenderAPI() const;

	virtual void HandleEvent(const WindowEvent& event);
	virtual void SwapBuffers();
	virtual void PollEvents();
	virtual void Close();

  protected:
	std::string m_name = "Unnamed Window";
	GLFWwindow* m_window = nullptr;
	glm::ivec2 m_resolution = {0, 0};
	RenderAPI m_renderAPI = RenderAPI::Undefined;
};

} // namespace PixieRenderer
