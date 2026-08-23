#include "UserInput.h"

#include <PixieRendering/Window/WindowEvent.h>

namespace PixieApp {

bool UserInput::buttonPressed = false;
int32_t UserInput::key = 0;
int32_t UserInput::scancode = 0;
int32_t UserInput::action = 0;
int32_t UserInput::mods = 0;

bool UserInput::mouseButtonPressed;
uint32_t UserInput::mouseButton;
uint32_t UserInput::mouseAction;
uint32_t UserInput::mouseMods;

double UserInput::mouseX = 0;
double UserInput::mouseY = 0;
double UserInput::mouseDeltaX = 0;
double UserInput::mouseDeltaY = 0;

double UserInput::mouseScrollX = 0;
double UserInput::mouseScrollY = 0;

void UserInput::Reset() {
	buttonPressed = false;
	mouseButtonPressed = false;
	mouseDeltaX = 0;
	mouseDeltaY = 0;
	mouseScrollX = 0;
	mouseScrollY = 0;
}

void UserInput::HandleEvent(const PixieRenderer::WindowEvent&) {
	// if (event.type == SDL_MOUSEMOTION) {
	// 	mouseX = event.motion.x;
	// 	mouseY = event.motion.y;
	// 	mouseDeltaX = event.motion.xrel;
	// 	mouseDeltaY = event.motion.yrel;
	// }
}

int32_t UserInput::GetKey(int32_t keyCode) {
	static const uint8_t state[256]{ 0 };
	return state[keyCode];
}

int32_t UserInput::GetMouseButton(int32_t mouseButtonIndex) {
	// int32_t x, y;
	// Uint32 buttons = SDL_GetMouseState(&x, &y);
	// if ((buttons & SDL_BUTTON(mouseButtonIndex)) != 0) {
	// 	return 1;
	// }
	return mouseButtonIndex;
}

} // namespace PixieApp
