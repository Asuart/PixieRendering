#pragma once
#include <chrono>

namespace PixieApp {

class Time {
  public:
	inline static std::chrono::milliseconds GetCurrentTime() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(
		    std::chrono::system_clock::now().time_since_epoch()
		);
	}

	inline static const float fixedDeltaTime = 0.002f;
	inline static float deltaTime = 0.0f;
	inline static std::chrono::milliseconds lastTime = Time::GetCurrentTime();

	static void Update();
};

} // namespace PixieApp
