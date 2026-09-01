#include "UIImageVulkan.h"

#include "../../dependencies/imgui/backends/imgui_impl_vulkan.h"

#include <PixieRendering/Renderer/Vulkan/RendererVulkan.h>

namespace PixieUI {

UIImageVulkan::UIImageVulkan(PixieRenderer::IRenderer* renderer) : m_renderer(renderer) {
}

UIImageVulkan::~UIImageVulkan() {
	if (m_displayTexture != VK_NULL_HANDLE) {
		ImGui_ImplVulkan_RemoveTexture(m_displayTexture);
		m_displayTexture = VK_NULL_HANDLE;
	}
}

void UIImageVulkan::SetTexture(PixieRenderer::TextureHandle texture) {
	if (m_displayTexture != VK_NULL_HANDLE) {
		ImGui_ImplVulkan_RemoveTexture(m_displayTexture);
		m_displayTexture = VK_NULL_HANDLE;
	}

	m_currentTexture = texture;
	m_currentFrameBuffer = {};

	if (texture) {
		auto* renderer = reinterpret_cast<PixieRenderer::RendererVulkan*>(m_renderer);
		m_displayTexture = ImGui_ImplVulkan_AddTexture(
		    renderer->GetTextureSampler(texture),
		    renderer->GetTextureImageView(texture),
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}
}

void UIImageVulkan::SetFrameBuffer(PixieRenderer::FrameBufferHandle frameBuffer) {
	if (m_displayTexture != VK_NULL_HANDLE) {
		ImGui_ImplVulkan_RemoveTexture(m_displayTexture);
		m_displayTexture = VK_NULL_HANDLE;
	}

	m_currentFrameBuffer = frameBuffer;
	m_currentTexture = {};

	if (frameBuffer) {
		auto* renderer = reinterpret_cast<PixieRenderer::RendererVulkan*>(m_renderer);
		m_displayTexture = ImGui_ImplVulkan_AddTexture(
		    renderer->GetFrameBufferSampler(frameBuffer),
		    renderer->GetFrameBufferColorImageView(frameBuffer),
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}
}

ImTextureID UIImageVulkan::GetTextureID() const {
	return reinterpret_cast<ImTextureID>(m_displayTexture);
}

} // namespace PixieUI
