#include "ApplicationStatsWindow.h"

#include <imgui.h>

#include <PixieApplication/Time/GlobalTimer.h>

using namespace PixieRenderer;
using namespace PixieApp;

namespace PixieUI {

ApplicationStatsWindow::ApplicationStatsWindow(UI* ui, IRenderer* renderer)
    : UIWindow(ui, renderer) {
}

void ApplicationStatsWindow::Draw() {
	ImGui::SetNextWindowSize(ImVec2(400, 400));
	if (ImGui::Begin("Stats", 0)) {
		for (const TimeMeasurement& timeMeasurement : GlobalTimer::GetTimers()) {
			double milli = (double)timeMeasurement.deltaTime.count() / 1000000.0f;
			std::string text = timeMeasurement.name + std::string(": ") + std::to_string(milli) +
			                   std::string("ms");
			ImGui::Text("%s", text.c_str());
		}
	}
	ImGui::End();
}

} // namespace PixieUI
