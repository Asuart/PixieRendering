#include "UI.h"

#include <vulkan/vulkan.hpp>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_vulkan.h>

#include <PixieRendering/Window/Window.h>
#include <PixieApplication/Log/Log.h>

#include "UIWindow.h"

namespace PixieUI {

UI::UI(PixieRenderer::Window* mainWindow, bool docking)
    :
	m_window(mainWindow), m_isDocking(docking) {
	IMGUI_CHECKVERSION();
}

UI::~UI() {
	for (size_t i = 0; i < m_windows.size(); i++) {
		delete m_windows[i];
	}
}

void UI::HandleEvent(const PixieRenderer::WindowEvent& event) {
	// ImGui_ImplSDL2_ProcessEvent(&event);
	for (PixieUI::UIWindow* window : m_windows) {
		window->HandleEvent(event);
	}
}

void UI::AddWindow(PixieUI::UIWindow* window) {
	if (!window) {
		return;
	}
	m_windows.push_back(window);
}

} // namespace PixieUI
