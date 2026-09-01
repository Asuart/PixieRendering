#pragma once
#include "UIImage.h"

#include <vulkan/vulkan.h>

#include <PixieRendering/Renderer/IRenderer.h>

namespace PixieUI {

class UIImageVulkan : public UIImage {
  public:
	explicit UIImageVulkan(PixieRenderer::IRenderer* renderer);
	virtual ~UIImageVulkan();

	void SetTexture(PixieRenderer::TextureHandle texture) override;
	void SetFrameBuffer(PixieRenderer::FrameBufferHandle frameBuffer) override;
	ImTextureID GetTextureID() const override;

  private:
	PixieRenderer::IRenderer* m_renderer;
	VkDescriptorSet m_displayTexture = VK_NULL_HANDLE;
	PixieRenderer::TextureHandle m_currentTexture = {};
	PixieRenderer::FrameBufferHandle m_currentFrameBuffer = {};
};

} // namespace PixieUI
