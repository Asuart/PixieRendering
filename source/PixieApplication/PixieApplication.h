#pragma once
#include <PixieRendering/PixieRendering.h>
#include <PixieRendering/RenderAPI.h>
#include <PixieRendering/Renderer/IRenderer.h>
#include <PixieRendering/Window/Window.h>

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
	virtual void BeforeDrawFrame() {
	}
	virtual void OnDrawFrame() {
	}
	virtual void AfterDrawFrame() {
	}
	virtual void HandleEvent(const PixieRenderer::WindowEvent&) {};
};

} // namespace PixieApp
