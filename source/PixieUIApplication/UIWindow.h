#pragma once
#include <PixieRendering/Renderer/IRenderer.h>
#include <PixieRendering/Window/WindowEvent.h>

namespace PixieUI {

class UIWindow {
public:
	UIWindow(PixieRenderer::IRenderer* renderer) : m_renderer(renderer) {};
	virtual ~UIWindow() {}

	virtual void Draw() = 0;
	virtual void HandleEvent(const PixieRenderer::WindowEvent&) {};

protected:
	PixieRenderer::IRenderer* m_renderer;
};

} // namespace PixieRenderer
