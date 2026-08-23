#include "DemoWindow.h"

#include <imgui.h>

using namespace PixieRenderer;

namespace PixieUI {

DemoWindow::DemoWindow(IRenderer* renderer) :
	UIWindow(renderer) {}

void DemoWindow::Draw() {
	ImGui::ShowDemoWindow();
}

}
