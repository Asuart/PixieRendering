#pragma once
#include "TimeMeasurement.h"

namespace PixieApp {

class Timer {
  public:
	Timer(const std::string& name);
	~Timer();

  private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace PixieApp
