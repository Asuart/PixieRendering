#include "TextureDisplayWindow.h"

#include <imgui.h>
#include <string>

using namespace PixieRenderer;

namespace PixieUI {

static const char* VERTEX_SHADER_SOURCE = R"(
#version 430 core

out vec2 fTexCoord;

uniform vec2 uPos;
uniform vec2 uSize;

const vec2 pos[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

const vec2 uv[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

void main() {
	fTexCoord = uv[gl_VertexID];
	vec2 transformedPosition = vec2(pos[gl_VertexID].x * uSize.x + uPos.x, -pos[gl_VertexID].y * uSize.y - uPos.y) * 2.0 - vec2(1.0, -1.0);
	gl_Position = vec4(transformedPosition, 0.0, 1.0);
}
)";

static const char* FRAGMENT_SHADER_SOURCE = R"(
#version 430 core

in vec2 fTexCoord;

out vec4 color;

uniform sampler2D displayTexture;

void main() {
	vec4 pixel = texture(displayTexture, fTexCoord);
	color = vec4(pixel.rgb, 1.0f);
}
)";

TextureDisplayWindow::TextureDisplayWindow(IRenderer* renderer, TextureHandle texture)
    :
	UIWindow(renderer),
	m_targetTexture(texture) {
	m_frameBuffer = m_renderer->CreateFrameBuffer({ 1280, 720 });

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

void TextureDisplayWindow::Draw() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin((std::string("Texture View##")).c_str())) {
		if (ImGui::IsWindowFocused()) {}

		ImVec2 viewportResolution = ImGui::GetContentRegionAvail();
		ImGui::SetNextWindowSize(viewportResolution);
		glm::ivec2 glmViewportResolution = { viewportResolution.x, viewportResolution.y };

		glm::vec2 pos(0.0f, 0.0f), size(1.0f, 1.0f);
		float textureAspect = Aspect(m_renderer->GetTextureResolution(m_targetTexture));
		float viewportAspect = Aspect(glmViewportResolution);
		if (viewportAspect > textureAspect) {
			size.x = textureAspect / viewportAspect;
			pos.x = (1.0f - size.x) * 0.5f;
		}
		else {
			size.y = viewportAspect / textureAspect;
			pos.y = (1.0f - size.y) * 0.5f;
		}

		m_renderer->ResizeFrameBuffer(m_frameBuffer, glmViewportResolution);
		m_renderer->BindFrameBuffer(m_frameBuffer);

		//m_renderer->SetUniform2f(m_shader, "uPos", pos);
		//m_renderer->SetUniform2f(m_shader, "uSize", size);
		m_renderer->BindTexture(m_shader, "displayTexture", m_targetTexture, 0);
		m_renderer->DrawMesh(m_screenPlane, m_shader);

		m_renderer->UnbindFrameBuffer();

		ImGui::Image((void*)m_renderer->GetInternalColorAttachmentID(m_frameBuffer), viewportResolution, { 0.0, 1.0 }, { 1.0, 0.0 });
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void TextureDisplayWindow::SetTexture(TextureHandle texture) {
	m_targetTexture = texture;
}

float TextureDisplayWindow::Aspect(glm::ivec2 resolution) {
    return static_cast<float>(resolution.x) / static_cast<float>(resolution.y);
}

}
