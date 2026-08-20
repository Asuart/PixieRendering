#pragma once
#include <glad/glad.h>

#include "MaterialOpenGL.h"
#include "ComputeProgramOpenGL.h"

namespace PixieRenderer {

MaterialOpenGL CompileShaderOpenGL(
    const char* vertexShaderSource,
    const char* framgentShaderSource,
    const char* geometryShaderSource = nullptr
);

ComputeShaderOpenGL CompileComputeShaderOpenGL(const char* computeShaderSource);

} // namespace PixieRenderer
