#pragma once
#include "PixieUIApplication/UIWindow.h"

#include <PixieRendering/Renderer/IRenderer.h>
#include <PixieRendering/Resources/ResourceHandles.h>

#include <vulkan/vulkan.h>

class WindowUI;

namespace PixieUI {

class TextureDisplayWindow : public UIWindow {
  public:
	TextureDisplayWindow(PixieRenderer::IRenderer* renderer, PixieRenderer::TextureHandle texture);
	TextureDisplayWindow(
	    PixieRenderer::IRenderer* renderer,
	    PixieRenderer::FrameBufferHandle frameBuffer
	);

	virtual void OnBeforeDraw() override;
	void Draw() override;

	void SetTexture(PixieRenderer::TextureHandle texture);
	void SetFrameBuffer(PixieRenderer::FrameBufferHandle frameBuffer);

  protected:
	PixieRenderer::FrameBufferHandle m_frameBuffer;
	PixieRenderer::TextureHandle m_targetTexture;
	PixieRenderer::FrameBufferHandle m_targetFrameBuffer;
	PixieRenderer::MaterialHandle m_shader;
	PixieRenderer::MeshHandle m_screenPlane;
	glm::uvec2 m_viewportResolution;
	VkDescriptorSet m_displayTexture = nullptr;

	float Aspect(glm::ivec2 resolution);
};

} // namespace PixieUI
