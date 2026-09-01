#pragma once
#include <imgui.h>

#include <PixieRendering/Resources/ResourceHandles.h>

namespace PixieUI {

class UIImage {
  public:
	virtual ~UIImage() = default;

	virtual void SetTexture(PixieRenderer::TextureHandle texture) = 0;
	virtual void SetFrameBuffer(PixieRenderer::FrameBufferHandle frameBuffer) = 0;
	virtual ImTextureID GetTextureID() const = 0;
};

} // namespace PixieUI
