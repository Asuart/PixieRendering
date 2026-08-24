#pragma once
#include <vector>

namespace PixieRenderer {
class Window;
struct WindowEvent;
} // namespace PixieRenderer

namespace PixieUI {

class UIWindow;

class UI {
  public:
	UI(PixieRenderer::Window* window, bool docking);
	virtual ~UI();

	virtual void HandleEvent(const PixieRenderer::WindowEvent& event);
	virtual void AddWindow(UIWindow* window);

	void OnBeforeDrawFrame();
	virtual void Draw() = 0;

  protected:
	PixieRenderer::Window* m_window;
	std::vector<UIWindow*> m_windows;
	bool m_isDocking;

  public:
	template <typename T> std::vector<T*> GetWindowOfType() {
		std::vector<T*> result;
		for (size_t i = 0; i < m_windows.size(); i++) {
			T* cast = dynamic_cast<T*>(m_windows[i]);
			if (cast) {
				result.push_back(cast);
			}
		}
		return result;
	}
};

} // namespace PixieUI
