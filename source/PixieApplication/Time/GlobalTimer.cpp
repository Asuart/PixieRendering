#include "GlobalTimer.h"

namespace PixieApp {

void GlobalTimer::StartTimer(const std::string& name) {
	TimeMeasurement& timer = GetTimer(name);
	timer.start = std::chrono::high_resolution_clock::now();
}

void GlobalTimer::StopTimer(const std::string& name) {
	TimeMeasurement& timer = GetTimer(name);
	timer.end = std::chrono::high_resolution_clock::now();
	timer.deltaTime = timer.end - timer.start;
}

TimeMeasurement& GlobalTimer::GetTimer(const std::string& name) {
	for (size_t i = 0; i < s_timers.size(); i++) {
		if (s_timers[i].name == name) {
			return s_timers[i];
		}
	}
	s_timers.push_back(TimeMeasurement{ name });
	return s_timers[s_timers.size() - 1];
}

const std::vector<TimeMeasurement>& GlobalTimer::GetTimers() {
	return s_timers;
}

} // namespace PixieApp
