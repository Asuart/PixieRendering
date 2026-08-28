#include "TextureDisplayWindow.h"

#include <imgui.h>
#include <string>

#include <PixieRendering/Renderer/Vulkan/RendererVulkan.h>

#include "../../dependencies/imgui/backends/imgui_impl_vulkan.h"

using namespace PixieRenderer;

namespace PixieUI {

static const char* VERTEX_SHADER_SOURCE = R"(
#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 boneIDs; 
layout(location = 4) in vec4 boneWeights; 

layout(location = 0) out vec2 fTexCoord;

layout(set = 0, binding = 0) uniform PlaneUBO {
	vec2 pos;
	vec2 size;
} plane;

void main() {
	fTexCoord = aTexCoord;
	vec2 transformedPosition = vec2(
		aPos.x * plane.size.x + plane.pos.x,
		-aPos.y * plane.size.y - plane.pos.y
		) * 2.0 - vec2(1.0, -1.0);
	gl_Position = vec4(transformedPosition, 0.0, 1.0);
}
)";

static const char* FRAGMENT_SHADER_SOURCE = R"(
#version 450 core

layout(location = 0) in vec2 fTexCoord;

layout(location = 0) out vec4 color;

// uniform sampler2D displayTexture;

void main() {
	// vec4 pixel = texture(displayTexture, fTexCoord);
	vec4 pixel = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	color = vec4(pixel.rgb, 1.0f);
}
)";

TextureDisplayWindow::TextureDisplayWindow(IRenderer* renderer, TextureHandle texture)
    : UIWindow(renderer), m_targetTexture(texture) {
	m_viewportResolution = { 1280, 720 };
	m_frameBuffer = m_renderer->CreateFrameBuffer(m_viewportResolution, TextureFormat::RGBA32f);

	SetTexture(texture);

	Material mat{ VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE };
	m_shader = m_renderer->CreateMaterial(&mat);

	Mesh mesh;
	mesh.vertexes = {
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	};
	mesh.indexes = { 0, 1, 2, 0, 2, 3 };

	m_screenPlane = m_renderer->CreateMesh(&mesh);
}


TextureDisplayWindow::TextureDisplayWindow(IRenderer* renderer, FrameBufferHandle frameBuffer)
    : UIWindow(renderer), m_targetFrameBuffer(frameBuffer) {
	m_viewportResolution = { 1280, 720 };
	m_frameBuffer = m_renderer->CreateFrameBuffer(m_viewportResolution, TextureFormat::RGBA32f);

	SetFrameBuffer(frameBuffer);

	Material mat{ VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE };
	m_shader = m_renderer->CreateMaterial(&mat);

	Mesh mesh;
	mesh.vertexes = {
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	};
	mesh.indexes = { 0, 1, 2, 0, 2, 3 };

	m_screenPlane = m_renderer->CreateMesh(&mesh);
}
void TextureDisplayWindow::OnBeforeDraw() {
	if (m_viewportResolution.x == 0 || m_viewportResolution.y == 0) {
		return;
	}

	m_renderer->ResizeFrameBuffer(m_frameBuffer, m_viewportResolution);
	m_renderer->BindFrameBuffer(m_frameBuffer);

	struct PlaceUBO {
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 size = { 1.0f, 1.0f };
	} planeUBO;

	float textureAspect = 1.0f;
	if (m_targetTexture) {
		textureAspect = Aspect(m_renderer->GetTextureResolution(m_targetTexture));
	} else if (m_frameBuffer) {
		textureAspect = Aspect(m_renderer->GetFrameBufferResolution(m_frameBuffer));
	}
	
	float viewportAspect = Aspect(m_viewportResolution);
	if (viewportAspect > textureAspect) {
		planeUBO.size.x = textureAspect / viewportAspect;
		planeUBO.position.x = (1.0f - planeUBO.size.x) * 0.5f;
	} else {
		planeUBO.size.y = viewportAspect / textureAspect;
		planeUBO.position.y = (1.0f - planeUBO.size.y) * 0.5f;
	}

	m_renderer->LoadUniformBuffer(m_shader, "PlaneUBO", &planeUBO, sizeof(PlaceUBO));
	//// m_renderer->BindTexture(m_shader, "displayTexture", m_targetTexture, 0);
	 m_renderer->DrawMesh(m_screenPlane, m_shader);

	m_renderer->UnbindFrameBuffer();
}

void TextureDisplayWindow::Draw() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin((std::string("Texture View##")).c_str())) {
		if (ImGui::IsWindowFocused()) {
		}

		ImVec2 viewportResolution = ImGui::GetContentRegionAvail();
		ImGui::SetNextWindowSize(viewportResolution);
		m_viewportResolution = { viewportResolution.x, viewportResolution.y };

		ImGui::Image(m_displayTexture, viewportResolution, { 0.0, 1.0 }, { 1.0, 0.0 });
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void TextureDisplayWindow::SetTexture(TextureHandle texture) {
	RendererVulkan* renderer = reinterpret_cast<RendererVulkan*>(m_renderer);
	m_displayTexture = ImGui_ImplVulkan_AddTexture(
	    renderer->GetTextureSampler(texture),
	    renderer->GetTextureImageView(texture),
	    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
}

void TextureDisplayWindow::SetFrameBuffer(FrameBufferHandle frameBuffer) {
	RendererVulkan* renderer = reinterpret_cast<RendererVulkan*>(m_renderer);
	m_displayTexture = ImGui_ImplVulkan_AddTexture(
	    renderer->GetFrameBufferSampler(frameBuffer),
	    renderer->GetFrameBufferColorImageView(frameBuffer),
	    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
}

float TextureDisplayWindow::Aspect(glm::ivec2 resolution) {
	return static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
}

} // namespace PixieUI
