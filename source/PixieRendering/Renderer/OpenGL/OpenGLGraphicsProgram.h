#pragma once
#include <string>
#include <unordered_map>

#include <glad/glad.h>

namespace PixieRenderer {

struct OpenGLGraphicsProgram {
  public:
	OpenGLGraphicsProgram() = default;
	explicit OpenGLGraphicsProgram(GLuint program);
	~OpenGLGraphicsProgram();

	OpenGLGraphicsProgram(const OpenGLGraphicsProgram&) = delete;
	OpenGLGraphicsProgram& operator=(const OpenGLGraphicsProgram&) = delete;
	OpenGLGraphicsProgram(OpenGLGraphicsProgram&& other) noexcept;
	OpenGLGraphicsProgram& operator=(OpenGLGraphicsProgram&& other) noexcept;

	void Bind();
	void BindTexture(const std::string& name, GLuint index);

	GLuint GetProgram() const {
		return m_id;
	}

	void BindUniformBlock(const std::string& name, GLuint bindingPoint);
	GLuint GetUniformBlockIndex(const std::string& name) const;

  private:
	GLuint m_id = 0;
	mutable std::unordered_map<std::string, GLuint> m_uniformBlockCache;
};

} // namespace PixieRenderer
