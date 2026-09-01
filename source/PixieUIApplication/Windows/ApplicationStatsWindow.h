#pragma once
#include "PixieUIApplication/UIWindow.h"

namespace PixieUI {

class ApplicationStatsWindow : public UIWindow {
  public:
	ApplicationStatsWindow(UI* ui, PixieRenderer::IRenderer* renderer);

	void Draw() override;
};

} // namespace PixieUI
