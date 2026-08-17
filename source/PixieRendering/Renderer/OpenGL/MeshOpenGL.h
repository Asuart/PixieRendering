#pragma once
#include "glad/glad.h"

namespace PixieRenderer {

struct MeshOpenGL {
	GLuint vertexArrayObject = 0;
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	GLuint indexesCount = 0;
};

} // namespace PixieRenderer
