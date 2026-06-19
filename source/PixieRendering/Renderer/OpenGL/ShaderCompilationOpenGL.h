#pragma once
#include <glad/glad.h>

struct ShaderOpenGL {
	GLuint id = 0;
};

struct ComputeShaderOpenGL {
	GLuint id = 0;
};

ShaderOpenGL CompileShaderOpenGL(const char* vertexShaderSource, const char* framgentShaderSource, const char* geometryShaderSource = nullptr);

ComputeShaderOpenGL CompileComputeShaderOpenGL(const char* computeShaderSource);
