#pragma once
#include "UIImage.h"

#include <PixieRendering/Renderer/IRenderer.h>

namespace PixieUI {

class UIImageOpenGL : public UIImage {
  public:
	explicit UIImageOpenGL(PixieRenderer::IRenderer* renderer);
	virtual ~UIImageOpenGL();

	void SetTexture(PixieRenderer::TextureHandle texture) override;
	void SetFrameBuffer(PixieRenderer::FrameBufferHandle frameBuffer) override;
	ImTextureID GetTextureID() const override;

  private:
	PixieRenderer::IRenderer* m_renderer;
	ImTextureID m_displayTexture = 0;
	PixieRenderer::TextureHandle m_currentTexture = {};
	PixieRenderer::FrameBufferHandle m_currentFrameBuffer = {};
};

} // namespace PixieUI
