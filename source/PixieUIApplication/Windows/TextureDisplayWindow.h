#pragma once
#include "PixieUIApplication/UIWindow.h"

#include <PixieRendering/ResourceHandles.h>
#include <PixieRendering/Renderer/IRenderer.h>

#include <vulkan/vulkan.h>

class WindowUI;

namespace PixieUI {

class TextureDisplayWindow : public UIWindow {
public:
	TextureDisplayWindow(PixieRenderer::IRenderer* renderer, PixieRenderer::TextureHandle texture);

	virtual void OnBeforeDraw() override;
	void Draw() override;

	void SetTexture(PixieRenderer::TextureHandle texture);

protected:
	PixieRenderer::FrameBufferHandle m_frameBuffer;
	PixieRenderer::TextureHandle m_targetTexture;
	PixieRenderer::MaterialHandle m_shader;
	PixieRenderer::MeshHandle m_screenPlane;
	glm::uvec2 m_viewportResolution;
	VkDescriptorSet m_displayTexture;

    float Aspect(glm::ivec2 resolution);
};

}
