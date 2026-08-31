#pragma once
#include <PixieApplication/PixieApplication.h>

#include "UI.h"

namespace PixieApp {

class PixieUIApplication : public PixieApplication {
public:
	PixieUIApplication(const std::string& name, glm::ivec2 resolution, PixieRenderer::RenderAPI renderAPI, bool docking);

	virtual void BeforeDrawFrame() override;
	void OnDrawFrame() final;
	void HandleEvent(const PixieRenderer::WindowEvent& event) final;

	virtual void OnBeforeDrawUI() {};
	virtual void HandleEventAfterUI(const PixieRenderer::WindowEvent&) {};

protected:
	PixieUI::UI* m_ui;
};

} // namespaec PixieAplication
