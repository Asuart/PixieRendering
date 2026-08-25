#include "PixieApplication.h"

#include <PixieRendering/PixieRendering.h>
#include <PixieRendering/RenderAPI.h>

#include "Time/ApplicationTime.h"
#include "Time/GlobalTimer.h"
#include "UserInput/UserInput.h"

using namespace PixieRenderer;

namespace PixieApp {

PixieApplication::PixieApplication(
    const std::string& name,
    glm::ivec2 resolution,
    RenderAPI renderAPI
)
    : m_renderAPI(renderAPI) {
	m_window = CreateWindow(name, resolution, renderAPI);
	if (!m_window) {
		throw "Failed to craete main window";
	}
	m_renderer = CreateRenderer(m_window);
	if (!m_renderer) {
		throw "Failed to create renderer";
	}
}

void PixieApplication::Start() {
	OnStart();
	while (!m_window->GetShouldClose()) {
		GlobalTimer::StartTimer("Main Loop");

		Time::Update();

		m_renderer->StartFrame();

		OnDrawFrame();
		if (m_renderAPI == RenderAPI::OpenGL) {
			m_window->SwapBuffers();
		}

		m_renderer->EndFrame();

		UserInput::Reset();
		m_window->PollEvents();

		//WindowEvent event;
		//while (false) {
		//	m_window->HandleEvent(event);
		//	UserInput::HandleEvent(event);
		//	HandleEvent(event);
		//}

		GlobalTimer::StopTimer("Main Loop");
	}
	OnClose();
};

} // namespace PixieApp
