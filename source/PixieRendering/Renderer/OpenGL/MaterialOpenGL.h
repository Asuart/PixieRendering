#pragma once
#include <unordered_map>
#include <string>

#include <glad/glad.h>

namespace PixieRenderer {

struct MaterialOpenGL {
	GLuint id = 0;
	std::unordered_map<std::string, uint32_t> nameToBindingMap;
};

} // namespace PixieRenderer
