#pragma once
#include <PixieRendering/Window/Window.h>
#include <PixieRendering/Renderer/IRenderer.h>
#include <PixieRendering/RenderAPI.h>

namespace PixieApp {

class PixieApplication {
  public:
	PixieApplication(
	    const std::string& name,
	    glm::ivec2 resolution,
	    PixieRenderer::RenderAPI renderAPI
	);

	virtual void Start();

  protected:
	PixieRenderer::RenderAPI m_renderAPI;
	PixieRenderer::IRenderer* m_renderer;
	PixieRenderer::Window* m_window;

	virtual void OnStart() {
	}
	virtual void OnClose() {
	}
	virtual void OnDrawFrame() {
	}
	virtual void HandleEvent(const PixieRenderer::WindowEvent&) {};
};

} // namespace PixieApp
