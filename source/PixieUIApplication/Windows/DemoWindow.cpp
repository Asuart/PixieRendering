#include "DemoWindow.h"

#include <imgui.h>

using namespace PixieRenderer;

namespace PixieUI {

DemoWindow::DemoWindow(UI* ui, IRenderer* renderer) : UIWindow(ui, renderer) {
}

void DemoWindow::Draw() {
	ImGui::ShowDemoWindow();
}

} // namespace PixieUI
