#pragma once
#include <vector>

#include "TimeMeasurement.h"

namespace PixieApp {

class GlobalTimer {
  public:
	static void StartTimer(const std::string& name);
	static void StopTimer(const std::string& name);
	static TimeMeasurement& GetTimer(const std::string& name);
	static const std::vector<TimeMeasurement>& GetTimers();

  private:
	inline static std::vector<TimeMeasurement> s_timers = {};
};

} // namespace PixeEngine
