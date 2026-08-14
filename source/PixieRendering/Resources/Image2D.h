#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "PixieRendering/TextureEnums.h"

namespace PixieRenderer {

struct Image2D {
	std::vector<uint8_t> data;
	glm::uvec2 resolution{0, 0};
	TextureFormat format = TextureFormat::Red8;
};

} // namespace PixieRenderer
