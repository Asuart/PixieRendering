#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "PixieRendering/TextureEnums.h"

constexpr GLint CastTextureWrapOpenGL(TextureWrap wrap) {
	switch (wrap) {
	case TextureWrap::Reapeat: return GL_REPEAT;
	case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
	case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
	case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
	default: return GL_REPEAT;
	}
}

constexpr GLint CastTextureFilteringOpenGL(TextureFiltering filtering) {
	switch (filtering) {
	case TextureFiltering::Linear: return GL_LINEAR;
	case TextureFiltering::Nearest: return GL_NEAREST;
	case TextureFiltering::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
	case TextureFiltering::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
	case TextureFiltering::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
	case TextureFiltering::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
	default: return GL_LINEAR;
	}
}

struct TextureOpenGL {
	GLuint id = 0;
	GLint internalFormat = GL_RED;
	glm::ivec2 resolution = { 0, 0 };
};
