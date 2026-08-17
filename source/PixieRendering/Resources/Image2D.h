#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "PixieRendering/TextureEnums.h"

namespace PixieRenderer {

struct Image2D {
	std::vector<uint8_t> pixels = {};
	glm::uvec2 resolution{0, 0};
	TextureFormat format = TextureFormat::Red8;
    TextureFiltering minFiltering = TextureFiltering::Linear;
    TextureFiltering magFiltering = TextureFiltering::Linear;
    TextureWrap wrapU = TextureWrap::Reapeat;
    TextureWrap wrapV = TextureWrap::Reapeat;
    TextureWrap wrapW = TextureWrap::Reapeat;
};

} // namespace PixieRenderer
