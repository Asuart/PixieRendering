#include "OpenGLGraphicsProgram.h"

namespace PixieRenderer {

OpenGLGraphicsProgram::OpenGLGraphicsProgram(GLuint program) : m_id(program) {
}

OpenGLGraphicsProgram::~OpenGLGraphicsProgram() {
	if (m_id) {
		glDeleteProgram(m_id);
		m_id = 0;
	}
}

OpenGLGraphicsProgram::OpenGLGraphicsProgram(OpenGLGraphicsProgram&& other) noexcept
    : m_id(other.m_id), m_uniformBlockCache(std::move(other.m_uniformBlockCache)) {
	other.m_id = 0;
}

OpenGLGraphicsProgram& OpenGLGraphicsProgram::operator=(OpenGLGraphicsProgram&& other) noexcept {
	if (this != &other) {
		if (m_id)
			glDeleteProgram(m_id);
		m_id = other.m_id;
		m_uniformBlockCache = std::move(other.m_uniformBlockCache);
		other.m_id = 0;
	}
	return *this;
}

void OpenGLGraphicsProgram::Bind() {
	glUseProgram(m_id);
}

void OpenGLGraphicsProgram::BindTexture(const std::string& name, GLuint index) {
	GLint loc = glGetUniformLocation(m_id, name.c_str());
	if (loc == -1) {
		return;
	}
	glUniform1i(loc, index);
}

GLuint OpenGLGraphicsProgram::GetUniformBlockIndex(const std::string& name) const {
	auto it = m_uniformBlockCache.find(name);
	if (it != m_uniformBlockCache.end())
		return it->second;

	GLuint index = glGetUniformBlockIndex(m_id, name.c_str());
	m_uniformBlockCache[name] = index;
	return index;
}

void OpenGLGraphicsProgram::BindUniformBlock(const std::string& name, GLuint bindingPoint) {
	GLuint index = GetUniformBlockIndex(name);
	if (index != GL_INVALID_INDEX) {
		glUniformBlockBinding(m_id, index, bindingPoint);
	}
}

} // namespace PixieRenderer
