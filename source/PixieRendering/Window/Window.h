#pragma once
#include <glm/glm.hpp>

#include "PixieRendering/Window/WindowEvent.h"
#include "PixieRendering/RenderAPI.h"

struct GLFWwindow;

namespace PixieRenderer {

class IRenderer;

class Window {
  public:
	Window(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI);
	virtual ~Window();

	void SetRenderer(IRenderer* renderer);

	glm::ivec2 GetResolution() const;
	bool GetShouldClose() const;
	RenderAPI GetRenderAPI() const;
	GLFWwindow* GetGLFWWindow() const;

	virtual void HandleEvent(const WindowEvent& event);
	virtual void SwapBuffers();
	virtual void PollEvents();
	virtual void Close();

	virtual void OnResize(glm::ivec2 newSize);

  protected:
	std::string m_name = "Unnamed Window";
	GLFWwindow* m_window = nullptr;
	IRenderer* m_renderer = nullptr;
	glm::ivec2 m_resolution = {0, 0};
	RenderAPI m_renderAPI = RenderAPI::Undefined;
};

} // namespace PixieRenderer
