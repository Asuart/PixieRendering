#pragma once
#include "glad/glad.h"

struct MeshOpenGL {
	GLuint vertexArrayObject = 0;
	GLuint positionsBuffer = 0;
	GLuint normalsBuffer = 0;
	GLuint texCoordsBuffer = 0;
	GLuint boneIDsBuffer = 0;
	GLuint boneWeightBuffer = 0;
	GLuint indicesBuffer = 0;
	GLuint indicesCount = 0;
};
