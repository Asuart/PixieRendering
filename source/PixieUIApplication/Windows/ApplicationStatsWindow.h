#pragma once
#include "PixieUIApplication/UIWindow.h"

namespace PixieUI {

class ApplicationStatsWindow : public UIWindow {
public:
	ApplicationStatsWindow(PixieRenderer::IRenderer* renderer);

	void Draw() override;
};

}
