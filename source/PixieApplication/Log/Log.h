#pragma once
#include <string>
#include <format>
#include <cstdio>

namespace PixieApp {

class Log {
  public:
	template <typename... Args> inline static void Message(const char* message, Args&&... args) {
		std::string formatted = std::vformat(message, std::make_format_args(args...));
		std::puts(formatted.c_str());
	}

	template <typename... Args> inline static void Warning(const char* warning, Args... args) {
		std::string formatted = std::vformat(warning, std::make_format_args(args...));
		std::puts(formatted.c_str());
	}

	template <typename... Args> inline static void Error(const char* error, Args... args) {
		std::string formatted = std::vformat(error, std::make_format_args(args...));
		std::puts(formatted.c_str());
	}
};

class DebugLog {
  public:
	template <typename... Args> static void Message(const char* message, Args... args) {
#ifndef NDEBUG
		std::string formatted = std::vformat(message, std::make_format_args(args...));
		std::puts(formatted.c_str());
#endif
	}

	template <typename... Args> static void Warning(const char* warning, Args... args) {
#ifndef NDEBUG
		std::string formatted = std::vformat(warning, std::make_format_args(args...));
		std::puts(formatted.c_str());
#endif
	}

	template <typename... Args> static void Error(const char* error, Args... args) {
#ifndef NDEBUG
		std::string formatted = std::vformat(error, std::make_format_args(args...));
		std::puts(formatted.c_str());
#endif
	}
};

} // namespace PixieApp
