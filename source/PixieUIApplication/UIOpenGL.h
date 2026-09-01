#pragma once
#include "UI.h"

namespace PixieRenderer {
class WindowOpenGL;
}

namespace PixieUI {

class UIOpenGL : public UI {
  public:
	UIOpenGL(PixieRenderer::WindowOpenGL* window, bool docking);
	~UIOpenGL();

	void Draw();
	UIImage* CreateUIImage(
	    PixieRenderer::IRenderer* renderer,
	    PixieRenderer::FrameBufferHandle handle
	) override;
	UIImage* CreateUIImage(PixieRenderer::IRenderer* renderer, PixieRenderer::TextureHandle handle)
	    override;
};

} // namespace PixieUI
