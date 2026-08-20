#include "RendererOpenGL.h"

#include <iostream>

#include "OpenGLCallbacks.h"

namespace PixieRenderer {

RendererOpenGL::RendererOpenGL(Window* mainWindow) : IRenderer(mainWindow, RenderAPI::OpenGL) {
	if (!gladLoadGL()) {
		std::cerr << "GLAD initialization failed\n";
		exit(2);
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(OpenglCallbackHandler, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

RendererOpenGL::~RendererOpenGL() {
}

void RendererOpenGL::StartFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::EndFrame() {
	assert(m_viewportStates.size() == 0);
}

MeshHandle RendererOpenGL::CreateMesh(const Mesh* mesh) {
	MeshOpenGL meshEntry;

	if (mesh != nullptr) {
		meshEntry.indexesCount = static_cast<GLuint>(mesh->indexes.size());

		glGenVertexArrays(1, &meshEntry.vertexArrayObject);
		glBindVertexArray(meshEntry.vertexArrayObject);

		if (mesh->indexes.size()) {
			glGenBuffers(1, &meshEntry.indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEntry.indexBuffer);
			glBufferData(
			    GL_ELEMENT_ARRAY_BUFFER,
			    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
			    &mesh->indexes[0],
			    GL_STATIC_DRAW
			);
		}

		if (mesh->vertexes.size()) {
			glGenBuffers(1, &meshEntry.vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, meshEntry.vertexBuffer);
			glBufferData(
			    GL_ARRAY_BUFFER,
			    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
			    &mesh->vertexes[0],
			    GL_STATIC_DRAW
			);

			glVertexAttribPointer(
			    0,
			    3,
			    GL_FLOAT,
			    GL_FALSE,
			    sizeof(Vertex),
			    (const void*)offsetof(Vertex, position)
			);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(
			    1,
			    3,
			    GL_FLOAT,
			    GL_FALSE,
			    sizeof(Vertex),
			    (const void*)offsetof(Vertex, normal)
			);
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(
			    2,
			    2,
			    GL_FLOAT,
			    GL_FALSE,
			    sizeof(Vertex),
			    (const void*)offsetof(Vertex, uv)
			);
			glEnableVertexAttribArray(2);

			glVertexAttribPointer(
			    3,
			    Vertex::cBonesPerVertex,
			    GL_INT,
			    GL_FALSE,
			    sizeof(Vertex),
			    (const void*)offsetof(Vertex, boneIDs)
			);
			glEnableVertexAttribArray(3);

			glVertexAttribPointer(
			    4,
			    Vertex::cBonesPerVertex,
			    GL_FLOAT,
			    GL_FALSE,
			    sizeof(Vertex),
			    (const void*)offsetof(Vertex, boneWeights)
			);
			glEnableVertexAttribArray(4);
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	m_meshes.push_back(meshEntry);

	return MeshHandle(static_cast<int32_t>(m_meshes.size() - 1));
}

void RendererOpenGL::DestroyMesh(MeshHandle handle) {
	MeshOpenGL& mesh = GetMeshEntry(handle);
	mesh.indexesCount = 0;
	glDeleteBuffers(1, &mesh.vertexBuffer);
	mesh.vertexBuffer = 0;
	glDeleteBuffers(1, &mesh.indexBuffer);
	mesh.indexBuffer = 0;
	glDeleteVertexArrays(1, &mesh.vertexArrayObject);
	mesh.vertexArrayObject = 0;
}

void RendererOpenGL::LoadMesh(MeshHandle handle, const Mesh* mesh) {
	MeshOpenGL& meshEntry = GetMeshEntry(handle);

	meshEntry.indexesCount = static_cast<GLuint>(mesh->indexes.size());

	glBindVertexArray(meshEntry.vertexArrayObject);

	if (mesh->indexes.size()) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEntry.indexBuffer);
		glBufferData(
		    GL_ELEMENT_ARRAY_BUFFER,
		    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
		    &mesh->indexes[0],
		    GL_STATIC_DRAW
		);
	}

	if (mesh->vertexes.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.vertexBuffer);
		glBufferData(
		    GL_ARRAY_BUFFER,
		    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
		    &mesh->vertexes[0],
		    GL_STATIC_DRAW
		);

		glVertexAttribPointer(
		    0,
		    3,
		    GL_FLOAT,
		    GL_FALSE,
		    sizeof(Vertex),
		    (const void*)offsetof(Vertex, position)
		);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(
		    1,
		    3,
		    GL_FLOAT,
		    GL_FALSE,
		    sizeof(Vertex),
		    (const void*)offsetof(Vertex, normal)
		);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(
		    2,
		    2,
		    GL_FLOAT,
		    GL_FALSE,
		    sizeof(Vertex),
		    (const void*)offsetof(Vertex, uv)
		);
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(
		    3,
		    Vertex::cBonesPerVertex,
		    GL_INT,
		    GL_FALSE,
		    sizeof(Vertex),
		    (const void*)offsetof(Vertex, boneIDs)
		);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(
		    4,
		    Vertex::cBonesPerVertex,
		    GL_FLOAT,
		    GL_FALSE,
		    sizeof(Vertex),
		    (const void*)offsetof(Vertex, boneWeights)
		);
		glEnableVertexAttribArray(4);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererOpenGL::DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) {
	const MaterialOpenGL& shaderEntry = GetShaderEntry(materialHandle);
	const MeshOpenGL& meshEntry = GetMeshEntry(meshHandle);
	glUseProgram(shaderEntry.id);
	glBindVertexArray(meshEntry.vertexArrayObject);
	glDrawElements(GL_TRIANGLES, meshEntry.indexesCount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);

	// void RendererOpenGL::BindShaderStorageBuffer(ShaderStorageBufferHandle handle, uint32_t
	// index) {
	// 	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);

	// 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, entry.id);
	// }

	// void RendererOpenGL::BindUniformBuffer(UniformBufferHandle handle, uint32_t index) {
	// 	UniformBufferOpenGL& entry = GetUniformBufferEntry(handle);

	// 	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	// 	glBindBufferBase(GL_UNIFORM_BUFFER, index, entry.id);
	// }
}

FrameBufferHandle RendererOpenGL::CreateFrameBuffer(glm::ivec2 resolution) {
	m_frameBuffers.push_back(FrameBufferOpenGL(resolution));
	return FrameBufferHandle(m_frameBuffers.size() - 1);
}

void RendererOpenGL::DestroyFrameBuffer(FrameBufferHandle handle) {
	FrameBufferOpenGL frameBuffer = GetFrameBufferEntry(handle);
	// TODO: destroy
}

void RendererOpenGL::ResizeFrameBuffer(FrameBufferHandle handle, glm::ivec2 resolution) {
	FrameBufferOpenGL& frameBufferEntry = GetFrameBufferEntry(handle);
	frameBufferEntry.Resize(resolution);
}

void RendererOpenGL::BindFrameBuffer(FrameBufferHandle handle) {
	FrameBufferOpenGL& frameBufferEntry = GetFrameBufferEntry(handle);
	StoreViewportState();
	frameBufferEntry.Bind();
	frameBufferEntry.ResizeViewport();
}

void RendererOpenGL::UnbindFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RestoreViewportState();
}

TextureHandle RendererOpenGL::CreateTexture(const Image2D* image) {
	TextureOpenGL textureEntry;

	if (image != nullptr) {
		textureEntry.resolution = image->resolution;

		glGenTextures(1, &textureEntry.id);
		glBindTexture(GL_TEXTURE_2D, textureEntry.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CastTextureWrapOpenGL(image->wrapU));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CastTextureWrapOpenGL(image->wrapV));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, CastTextureWrapOpenGL(image->wrapW));
		glTexParameteri(
		    GL_TEXTURE_2D,
		    GL_TEXTURE_MIN_FILTER,
		    CastTextureFilteringOpenGL(image->minFiltering)
		);
		glTexParameteri(
		    GL_TEXTURE_2D,
		    GL_TEXTURE_MAG_FILTER,
		    CastTextureFilteringOpenGL(image->magFiltering)
		);

		switch (image->format) {
		case TextureFormat::Red8:
			textureEntry.internalFormat = GL_RED;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RED,
			    GL_UNSIGNED_BYTE,
			    image->pixels.data()
			);
			break;
		case TextureFormat::RGB8:
			textureEntry.internalFormat = GL_RGB;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RGB,
			    GL_UNSIGNED_BYTE,
			    image->pixels.data()
			);
			break;
		case TextureFormat::RGBA8:
			textureEntry.internalFormat = GL_RGBA;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RGBA,
			    GL_UNSIGNED_BYTE,
			    image->pixels.data()
			);
			break;
		case TextureFormat::Red32f:
			textureEntry.internalFormat = GL_R32F;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RED,
			    GL_FLOAT,
			    image->pixels.data()
			);
			break;
		case TextureFormat::RGB32f:
			textureEntry.internalFormat = GL_RGB32F;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RGB,
			    GL_FLOAT,
			    image->pixels.data()
			);
			break;
		case TextureFormat::RGBA32f:
			textureEntry.internalFormat = GL_RGBA32F;
			glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    textureEntry.internalFormat,
			    image->resolution.x,
			    image->resolution.y,
			    0,
			    GL_RGBA,
			    GL_FLOAT,
			    image->pixels.data()
			);
			break;
		default:
			throw "RendererOpenGL::CreateTexture: unhandled texture type";
		}
	}

	m_textures.push_back(textureEntry);

	return TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
}

void RendererOpenGL::DestroyTexture(TextureHandle handle) {
	TextureOpenGL& texture = GetTextureEntry(handle);
	glDeleteTextures(1, &texture.id);
	texture.id = 0;
}

void RendererOpenGL::LoadTexture(TextureHandle handle, const Image2D* image) {
	TextureOpenGL& entry = GetTextureEntry(handle);
	entry.resolution = image->resolution;

	switch (image->format) {
	case TextureFormat::Red8:
		entry.internalFormat = GL_RED;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RED,
		    GL_UNSIGNED_BYTE,
		    image->pixels.data()
		);
		break;
	case TextureFormat::RGB8:
		entry.internalFormat = GL_RGB;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RGB,
		    GL_UNSIGNED_BYTE,
		    image->pixels.data()
		);
		break;
	case TextureFormat::RGBA8:
		entry.internalFormat = GL_RGBA;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    image->pixels.data()
		);
		break;
	case TextureFormat::Red32f:
		entry.internalFormat = GL_R32F;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RED,
		    GL_FLOAT,
		    image->pixels.data()
		);
		break;
	case TextureFormat::RGB32f:
		entry.internalFormat = GL_RGB32F;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RGB,
		    GL_FLOAT,
		    image->pixels.data()
		);
		break;
	case TextureFormat::RGBA32f:
		entry.internalFormat = GL_RGBA32F;
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    entry.internalFormat,
		    image->resolution.x,
		    image->resolution.y,
		    0,
		    GL_RGBA,
		    GL_FLOAT,
		    image->pixels.data()
		);
		break;
	default:
		throw "RendererOpenGL::CreateTexture: unhandled texture type";
	}
}

void RendererOpenGL::SetTextureFiltering(
    TextureHandle handle,
    TextureFiltering minFilter,
    TextureFiltering magFilter
) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CastTextureFilteringOpenGL(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CastTextureFilteringOpenGL(magFilter));
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RendererOpenGL::SetTextureWrap(
    TextureHandle handle,
    TextureWrap wrapU,
    TextureWrap wrapV,
    TextureWrap wrapW
) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CastTextureWrapOpenGL(wrapU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CastTextureWrapOpenGL(wrapV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, CastTextureWrapOpenGL(wrapW));
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RendererOpenGL::GenerateTextureMipmaps(TextureHandle handle) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

glm::ivec2 RendererOpenGL::GetTextureResolution(TextureHandle handle) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	return texture.resolution;
}

void RendererOpenGL::BindTexture(
    MaterialHandle materialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	TextureOpenGL textureEntry = GetTextureEntry(textureHandle);
	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
	glBindTexture(GL_TEXTURE_2D, textureEntry.id);
	// SetUniform1i(materialHandle, name, static_cast<int32_t>(index));
	(void)name;
	(void)materialHandle;
}

void RendererOpenGL::BindTexture(
    ComputeProgramHandle computeMaterialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	TextureOpenGL textureEntry = GetTextureEntry(textureHandle);
	glBindImageTexture(
	    static_cast<GLuint>(index),
	    textureEntry.id,
	    0,
	    GL_FALSE,
	    0,
	    GL_READ_WRITE,
	    textureEntry.internalFormat
	);
	// SetUniform1i(computeMaterialHandle, name, static_cast<int32_t>(index));
	(void)name;
	(void)computeMaterialHandle;
}

ShaderStorageBufferHandle
RendererOpenGL::CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) {
	ShaderStorageBufferOpenGL entry;
	entry.size = size;
	glGenBuffers(1, &entry.id);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);

	m_shaderStorageBuffers.push_back(entry);
	return ShaderStorageBufferHandle(m_shaderStorageBuffers.size() - 1);
}

void RendererOpenGL::DestroyShaderStorageBuffer(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferOpenGL& buffer = GetShaderStorageBufferEntry(handle);
	glDeleteBuffers(1, &buffer.id);
	buffer.id = 0;
	buffer.size = 0;
}

void RendererOpenGL::LoadShaderStorageBuffer(
    ShaderStorageBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	entry.size = size;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);
}

uint32_t RendererOpenGL::GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	return entry.size;
}

std::vector<uint8_t> RendererOpenGL::GetShaderStorageBufferData(
    ShaderStorageBufferHandle handle,
    uint32_t offset,
    uint32_t size
) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	std::vector<uint8_t> values(size);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, (GLvoid*)values.data());

	return values;
}

UniformBufferHandle RendererOpenGL::CreateUniformBuffer(const uint8_t* data, uint32_t size) {
	UniformBufferOpenGL entry;
	entry.size = size;
	glGenBuffers(1, &entry.id);

	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	glBufferData(GL_UNIFORM_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);

	m_uniformBuffers.push_back(entry);
	return UniformBufferHandle(m_uniformBuffers.size() - 1);
}

void RendererOpenGL::DestroyUniformBuffer(UniformBufferHandle handle) {
	UniformBufferOpenGL& buffer = GetUniformBufferEntry(handle);
	glDeleteBuffers(1, &buffer.id);
	buffer.id = 0;
	buffer.size = 0;
}

void RendererOpenGL::LoadUniformBuffer(
    UniformBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	UniformBufferOpenGL& entry = GetUniformBufferEntry(handle);
	entry.size = size;

	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	glBufferData(GL_UNIFORM_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);
}

void RendererOpenGL::LoadUniformBuffer(
    MaterialHandle materialHandle,
    const std::string& name,
    const void* data,
    size_t size
) {
	MaterialOpenGL& material = GetShaderEntry(materialHandle);
	if (!material.nameToBindingMap.contains(name)) {
		std::cout << "LoadUniformBuffer: shader doesn't have binding '" << name << "'\n";
		return;
	}
	uint32_t binding = material.nameToBindingMap[name];
	glBindBuffer(GL_UNIFORM_BUFFER, binding);
	glBufferData(GL_UNIFORM_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);
}

MaterialHandle RendererOpenGL::CreateMaterial(const Material* materialInfo) {
	MaterialOpenGL shaderEntry =
	    CompileShaderOpenGL(materialInfo->vertexShaderSource, materialInfo->fragmentShaderSource);
	m_shaders.push_back(shaderEntry);
	return MaterialHandle(m_shaders.size() - 1);
}

void RendererOpenGL::DestroyMaterial(MaterialHandle handle) {
	MaterialOpenGL& shader = GetShaderEntry(handle);
	glDeleteProgram(shader.id);
	shader.id = 0;
}

ComputeProgramHandle RendererOpenGL::CreateComputeProgram(const char* source) {
	ComputeShaderOpenGL computeShaderEntry = CompileComputeShaderOpenGL(source);
	m_computeShaders.push_back(computeShaderEntry);
	return ComputeProgramHandle(m_computeShaders.size() - 1);
}

void RendererOpenGL::DestroyComputeProgram(ComputeProgramHandle handle) {
	ComputeShaderOpenGL computeProgram = GetComputeShaderEntry(handle);
	glDeleteProgram(computeProgram.id);
	computeProgram.id = 0;
}

void RendererOpenGL::DispatchComputeProgram(
    ComputeProgramHandle handle,
    int32_t x,
    int32_t y,
    int32_t z
) {
	ComputeShaderOpenGL computeShaderEntry = GetComputeShaderEntry(handle);
	glUseProgram(computeShaderEntry.id);
	glDispatchCompute(x, y, z);
	glUseProgram(0);
}

void RendererOpenGL::SetViewport(glm::ivec2 start, glm::ivec2 resolution) {
	glViewport(start.x, start.y, resolution.x, resolution.y);
}

void RendererOpenGL::WaitIdle() {
	glFinish();
}

void RendererOpenGL::MemoryBarriersAll() {
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

uint64_t RendererOpenGL::GetInternalID(TextureHandle handle) {
	TextureOpenGL& textureEntry = GetTextureEntry(handle);
	return textureEntry.id;
}

uint64_t RendererOpenGL::GetInternalColorAttachmentID(FrameBufferHandle handle) {
	FrameBufferOpenGL& frameBufferEntry = GetFrameBufferEntry(handle);
	return frameBufferEntry.GetColorHandle();
}

uint64_t RendererOpenGL::GetInternalDepthAttachmentID(FrameBufferHandle handle) {
	FrameBufferOpenGL& frameBufferEntry = GetFrameBufferEntry(handle);
	return frameBufferEntry.GetDepthHandle();
}

TextureOpenGL& RendererOpenGL::GetTextureEntry(TextureHandle handle) {
	return m_textures[handle.id];
}

MeshOpenGL& RendererOpenGL::GetMeshEntry(MeshHandle handle) {
	return m_meshes[handle.id];
}

FrameBufferOpenGL& RendererOpenGL::GetFrameBufferEntry(FrameBufferHandle handle) {
	return m_frameBuffers[handle.id];
}

MaterialOpenGL& RendererOpenGL::GetShaderEntry(MaterialHandle handle) {
	return m_shaders[handle.id];
}

ComputeShaderOpenGL& RendererOpenGL::GetComputeShaderEntry(ComputeProgramHandle handle) {
	return m_computeShaders[handle.id];
}

ShaderStorageBufferOpenGL&
RendererOpenGL::GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle) {
	return m_shaderStorageBuffers[handle.id];
}

UniformBufferOpenGL& RendererOpenGL::GetUniformBufferEntry(UniformBufferHandle handle) {
	return m_uniformBuffers[handle.id];
}

void RendererOpenGL::StoreViewportState() {
	GLint originalViewport[4];
	glGetIntegerv(GL_VIEWPORT, originalViewport);
	ViewportStateOpenGL state;
	state.x = originalViewport[0];
	state.y = originalViewport[1];
	state.width = originalViewport[2];
	state.height = originalViewport[3];
	m_viewportStates.push_back(state);
}

void RendererOpenGL::RestoreViewportState() {
	ViewportStateOpenGL state = m_viewportStates.back();
	m_viewportStates.pop_back();
	glViewport(state.x, state.y, state.width, state.height);
}

} // namespace PixieRenderer
