#include "PixieUIApplication.h"

#include "UIOpenGL.h"
#include "UIVulkan.h"

using namespace PixieRenderer;

namespace PixieApp {

PixieUIApplication::PixieUIApplication(const std::string& name, glm::ivec2 resolution, RenderAPI renderAPI, bool docking) :
	PixieApplication(name, resolution, renderAPI) {
	switch (m_renderAPI) {
	case RenderAPI::OpenGL: {
		WindowOpenGL* mainWindowOpenGL = reinterpret_cast<WindowOpenGL*>(m_window);
		m_ui = new PixieUI::UIOpenGL(mainWindowOpenGL, docking);
		break;
	}
	case RenderAPI::Vulkan: {
		WindowVulkan* mainWindowVulkan = reinterpret_cast<WindowVulkan*>(m_window);
		m_ui = new PixieUI::UIVulkan(mainWindowVulkan, docking);
		break;
	}
	default:
		throw "UI initialization failed";
	}
}

void PixieUIApplication::OnDrawFrame() {
	m_ui->OnBeforeDrawFrame();
	m_renderer->BeginRenderPass();
	m_ui->Draw();
	m_renderer->EndRenderPass();
}

void PixieUIApplication::HandleEvent(const WindowEvent& event) {
	m_ui->HandleEvent(event);
	HandleEventAfterUI(event);
}

}
