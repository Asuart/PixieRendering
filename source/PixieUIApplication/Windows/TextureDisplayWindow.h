#pragma once
#include "PixieUIApplication/UIWindow.h"

#include <PixieRendering/ResourceHandles.h>
#include <PixieRendering/Renderer/IRenderer.h>

class WindowUI;

namespace PixieUI {

class TextureDisplayWindow : public UIWindow {
public:
	TextureDisplayWindow(PixieRenderer::IRenderer* renderer, PixieRenderer::TextureHandle texture);

	void Draw() override;

	void SetTexture(PixieRenderer::TextureHandle texture);

protected:
	PixieRenderer::FrameBufferHandle m_frameBuffer;
	PixieRenderer::TextureHandle m_targetTexture;
	PixieRenderer::MaterialHandle m_shader;
	PixieRenderer::MeshHandle m_screenPlane;

    float Aspect(glm::ivec2 resolution);
};

}
