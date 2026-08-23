#pragma once
#include "UI.h"

namespace PixieRenderer {
class WindowVulkan;
}

namespace PixieUI {

class UIVulkan : public UI {
  public:
	UIVulkan(PixieRenderer::WindowVulkan* window, bool docking);
	~UIVulkan();

	void Draw();
};

} // namespace PixieUI
