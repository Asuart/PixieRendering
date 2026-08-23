#pragma once
#include <string>
#include <chrono>

namespace PixieApp {

struct TimeMeasurement {
	std::string name;
	std::chrono::high_resolution_clock::time_point start{};
	std::chrono::high_resolution_clock::time_point end{};
	std::chrono::nanoseconds deltaTime{};
};

} // namespace PixieApp
