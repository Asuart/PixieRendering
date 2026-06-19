#pragma once
#include <cstdint>
#include <string>

enum class RenderAPI : uint32_t {
	Undefined = 0,
	OpenGL,
	Vulkan,
	COUNT
};

inline std::string to_string(RenderAPI api) {
	switch (api) {
        case RenderAPI::OpenGL: return "OpenGL";
        case RenderAPI::Vulkan: return "Vulkan";
        default: return "<undefined>";
	}
}
