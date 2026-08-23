#pragma once
#include "PixieUIApplication/UIWindow.h"

namespace PixieUI {

class DemoWindow : public UIWindow {
public:
	DemoWindow(PixieRenderer::IRenderer* renderer);

	void Draw() override;
};

}
