#include "RendererOpenGL.h"

#include <iostream>

#include "OpenGLCallbacks.h"

RendererOpenGL::RendererOpenGL(MainWindow* mainWindow) :
	Renderer(mainWindow, RenderAPI::OpenGL) {
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

RendererOpenGL::~RendererOpenGL() {}

void RendererOpenGL::StartFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::EndFrame() {
	assert(m_viewportStates.size() == 0);
}

MeshHandle RendererOpenGL::CreateMesh() {
	MeshOpenGL meshEntry;
	m_meshes.push_back(meshEntry);
	return MeshHandle(static_cast<int32_t>(m_meshes.size() - 1));
}

MeshHandle RendererOpenGL::LoadMesh(const Mesh* mesh) {
	MeshOpenGL meshEntry;
	meshEntry.indicesCount = static_cast<GLuint>(mesh->m_indices.size());

	glGenVertexArrays(1, &meshEntry.vertexArrayObject);
	glBindVertexArray(meshEntry.vertexArrayObject);

	if (mesh->m_indices.size()) {
		glGenBuffers(1, &meshEntry.indicesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEntry.indicesBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->m_indices[0]) * mesh->m_indices.size(), &mesh->m_indices[0], GL_STATIC_DRAW);
	}

	if (mesh->m_positions.size()) {
		glGenBuffers(1, &meshEntry.positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.positionsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_positions[0]) * mesh->m_positions.size(), &mesh->m_positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh->m_positions[0]), 0);
		glEnableVertexAttribArray(0);
	}

	if (mesh->m_normals.size()) {
		glGenBuffers(1, &meshEntry.normalsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.normalsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_normals[0]) * mesh->m_normals.size(), &mesh->m_normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh->m_normals[0]), 0);
		glEnableVertexAttribArray(1);
	}

	if (mesh->m_texCoords.size()) {
		glGenBuffers(1, &meshEntry.texCoordsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.texCoordsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_texCoords[0]) * mesh->m_texCoords.size(), &mesh->m_texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh->m_texCoords[0]), 0);
		glEnableVertexAttribArray(2);
	}

	if (mesh->m_boneIDs.size()) {
		glGenBuffers(1, &meshEntry.boneIDsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.boneIDsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_boneIDs[0]) * mesh->m_boneIDs.size(), &mesh->m_boneIDs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, mesh->m_bonesPerVertice, GL_INT, GL_FALSE, sizeof(mesh->m_boneIDs[0]) * mesh->m_bonesPerVertice, 0);
		glEnableVertexAttribArray(3);
	}

	if (mesh->m_boneWeights.size()) {
		glGenBuffers(1, &meshEntry.boneWeightBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.boneWeightBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_boneWeights[0]) * mesh->m_boneWeights.size(), &mesh->m_boneWeights[0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, mesh->m_bonesPerVertice, GL_FLOAT, GL_FALSE, sizeof(mesh->m_boneWeights[0]) * mesh->m_bonesPerVertice, 0);
		glEnableVertexAttribArray(4);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_meshes.push_back(meshEntry);

	return MeshHandle(static_cast<int32_t>(m_meshes.size() - 1));
}

void RendererOpenGL::LoadMesh(MeshHandle& handle, const Mesh* mesh) {
	MeshOpenGL& meshEntry = GetMeshEntry(handle);

	meshEntry.indicesCount = static_cast<GLuint>(mesh->m_indices.size());

	glBindVertexArray(meshEntry.vertexArrayObject);

	if (mesh->m_indices.size()) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEntry.indicesBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->m_indices[0]) * mesh->m_indices.size(), &mesh->m_indices[0], GL_STATIC_DRAW);
	}

	if (mesh->m_positions.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.positionsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_positions[0]) * mesh->m_positions.size(), &mesh->m_positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh->m_positions[0]), 0);
		glEnableVertexAttribArray(0);
	}

	if (mesh->m_normals.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.normalsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_normals[0]) * mesh->m_normals.size(), &mesh->m_normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh->m_normals[0]), 0);
		glEnableVertexAttribArray(1);
	}

	if (mesh->m_texCoords.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.texCoordsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_texCoords[0]) * mesh->m_texCoords.size(), &mesh->m_texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh->m_texCoords[0]), 0);
		glEnableVertexAttribArray(2);
	}

	if (mesh->m_boneIDs.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.boneIDsBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_boneIDs[0]) * mesh->m_boneIDs.size(), &mesh->m_boneIDs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, mesh->m_bonesPerVertice, GL_INT, GL_FALSE, sizeof(mesh->m_boneIDs[0]) * mesh->m_bonesPerVertice, 0);
		glEnableVertexAttribArray(3);
	}

	if (mesh->m_boneWeights.size()) {
		glBindBuffer(GL_ARRAY_BUFFER, meshEntry.boneWeightBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->m_boneWeights[0]) * mesh->m_boneWeights.size(), &mesh->m_boneWeights[0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, mesh->m_bonesPerVertice, GL_FLOAT, GL_FALSE, sizeof(mesh->m_boneWeights[0]) * mesh->m_bonesPerVertice, 0);
		glEnableVertexAttribArray(4);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererOpenGL::DrawMesh(MeshHandle meshHandle, ShaderHandle shaderHandle) {
	const ShaderOpenGL& shaderEntry = GetShaderEntry(shaderHandle);
	const MeshOpenGL& meshEntry = GetMeshEntry(meshHandle);
	glUseProgram(shaderEntry.id);
	glBindVertexArray(meshEntry.vertexArrayObject);
	glDrawElements(GL_TRIANGLES, meshEntry.indicesCount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);
}

FrameBufferHandle RendererOpenGL::CreateFrameBuffer(glm::ivec2 resolution) {
	m_frameBuffers.push_back(FrameBufferOpenGL(resolution));
	return FrameBufferHandle(m_frameBuffers.size() - 1);
}

void RendererOpenGL::ResizeFrameBuffer(FrameBufferHandle& handle, glm::ivec2 resolution) {
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

void RendererOpenGL::ClearFrameBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::ResizeViewport(glm::ivec2 resolution) {
	glViewport(0, 0, resolution.x, resolution.y);
}

TextureHandle RendererOpenGL::CreateTexture(glm::ivec2 resolution, TextureFormat format) {
	TextureOpenGL textureEntry;
	textureEntry.resolution = resolution;

	glGenTextures(1, &textureEntry.id);
	glBindTexture(GL_TEXTURE_2D, textureEntry.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch (format) {
	case TextureFormat::Red8:
		textureEntry.internalFormat = GL_RED;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		break;
	case TextureFormat::RGB8:
		textureEntry.internalFormat = GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		break;
	case TextureFormat::RGBA8:
		textureEntry.internalFormat = GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		break;
	case TextureFormat::Red32f:
		textureEntry.internalFormat = GL_R32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, nullptr);
		break;
	case TextureFormat::RGB32f:
		textureEntry.internalFormat = GL_RGB32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, nullptr);
		break;
	case TextureFormat::RGBA32f:
		textureEntry.internalFormat = GL_RGBA32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);
		break;
	default:
		throw "RendererOpenGL::CreateTexture: unhandled texture type";
	}

	m_textures.push_back(textureEntry);

	return TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
}

TextureHandle RendererOpenGL::LoadTexture(const Image2D* image) {
	TextureOpenGL textureEntry;
	textureEntry.resolution = image->resolution;

	glGenTextures(1, &textureEntry.id);
	glBindTexture(GL_TEXTURE_2D, textureEntry.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch (image->format) {
	case TextureFormat::Red8:
		textureEntry.internalFormat = GL_RED;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::RGB8:
		textureEntry.internalFormat = GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::RGBA8:
		textureEntry.internalFormat = GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::Red32f:
		textureEntry.internalFormat = GL_R32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RED, GL_FLOAT, image->data.data());
		break;
	case TextureFormat::RGB32f:
		textureEntry.internalFormat = GL_RGB32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGB, GL_FLOAT, image->data.data());
		break;
	case TextureFormat::RGBA32f:
		textureEntry.internalFormat = GL_RGBA32F;
		glTexImage2D(GL_TEXTURE_2D, 0, textureEntry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGBA, GL_FLOAT, image->data.data());
		break;
	default:
		throw "RendererOpenGL::CreateTexture: unhandled texture type";
	}

	m_textures.push_back(textureEntry);

	return TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
}

void RendererOpenGL::LoadTexture(TextureHandle& handle, const Image2D* image) {
	TextureOpenGL& entry = GetTextureEntry(handle);
	entry.resolution = image->resolution;

	switch (image->format) {
	case TextureFormat::Red8:
		entry.internalFormat = GL_RED;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::RGB8:
		entry.internalFormat = GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::RGBA8:
		entry.internalFormat = GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data.data());
		break;
	case TextureFormat::Red32f:
		entry.internalFormat = GL_R32F;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RED, GL_FLOAT, image->data.data());
		break;
	case TextureFormat::RGB32f:
		entry.internalFormat = GL_RGB32F;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGB, GL_FLOAT, image->data.data());
		break;
	case TextureFormat::RGBA32f:
		entry.internalFormat = GL_RGBA32F;
		glTexImage2D(GL_TEXTURE_2D, 0, entry.internalFormat, image->resolution.x, image->resolution.y, 0, GL_RGBA, GL_FLOAT, image->data.data());
		break;
	default:
		throw "RendererOpenGL::CreateTexture: unhandled texture type";
	}
}

void RendererOpenGL::SetTextureFiltering(TextureHandle handle, TextureFiltering minFilter, TextureFiltering magFilter) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CastTextureFilteringOpenGL(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CastTextureFilteringOpenGL(magFilter));
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RendererOpenGL::SetTextureWrap(TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) {
	const TextureOpenGL& texture = GetTextureEntry(handle);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CastTextureWrapOpenGL(wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CastTextureWrapOpenGL(wrapT));
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

ShaderStorageBufferHandle RendererOpenGL::LoadShaderStorageBuffer(uint8_t* data, uint32_t size) {
	ShaderStorageBufferOpenGL entry;
	entry.size = size;
	glGenBuffers(1, &entry.id);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);

	m_shaderStorageBuffers.push_back(entry);
	return ShaderStorageBufferHandle(m_shaderStorageBuffers.size() - 1);
}

void RendererOpenGL::LoadShaderStorageBuffer(ShaderStorageBufferHandle handle, uint8_t* data, uint32_t size) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	entry.size = size;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);
}

void RendererOpenGL::BindShaderStorageBuffer(ShaderStorageBufferHandle handle, uint32_t index) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, entry.id);
}

uint32_t RendererOpenGL::GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	return entry.size;
}

std::vector<uint8_t> RendererOpenGL::GetShaderStorageBufferData(ShaderStorageBufferHandle handle, uint32_t offset, uint32_t size) {
	ShaderStorageBufferOpenGL& entry = GetShaderStorageBufferEntry(handle);
	std::vector<uint8_t> values(size);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, entry.id);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, (GLvoid*)values.data());

	return values;
}

UniformBufferHandle RendererOpenGL::LoadUniformBuffer(uint8_t* data, uint32_t size) {
	UniformBufferOpenGL entry;
	entry.size = size;
	glGenBuffers(1, &entry.id);

	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	glBufferData(GL_UNIFORM_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);

	m_uniformBuffers.push_back(entry);
	return UniformBufferHandle(m_uniformBuffers.size() - 1);
}

void RendererOpenGL::LoadUniformBuffer(UniformBufferHandle handle, uint8_t* data, uint32_t size) {
	UniformBufferOpenGL& entry = GetUniformBufferEntry(handle);
	entry.size = size;

	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	glBufferData(GL_UNIFORM_BUFFER, size, (GLvoid*)data, GL_DYNAMIC_DRAW);
}

void RendererOpenGL::BindUniformBuffer(UniformBufferHandle handle, uint32_t index) {
	UniformBufferOpenGL& entry = GetUniformBufferEntry(handle);

	glBindBuffer(GL_UNIFORM_BUFFER, entry.id);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, entry.id);
}

ShaderHandle RendererOpenGL::CreateShader(const std::string& vertexShaderSource, const std::string fragmentShaderShource) {
	ShaderOpenGL shaderEntry = CompileShaderOpenGL(vertexShaderSource.c_str(), fragmentShaderShource.c_str());
	m_shaders.push_back(shaderEntry);
	return ShaderHandle(m_shaders.size() - 1);
}

void RendererOpenGL::BindTexture(ShaderHandle shaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index) {
	TextureOpenGL textureEntry = GetTextureEntry(textureHandle);
	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
	glBindTexture(GL_TEXTURE_2D, textureEntry.id);
	//SetUniform1i(shaderHandle, name, static_cast<int32_t>(index));
	(void)name;
	(void)shaderHandle;
}

ComputeShaderHandle RendererOpenGL::CreateComputeShader(const std::string& source) {
	ComputeShaderOpenGL computeShaderEntry = CompileComputeShaderOpenGL(source.c_str());
	m_computeShaders.push_back(computeShaderEntry);
	return ComputeShaderHandle(m_computeShaders.size() - 1);
}

void RendererOpenGL::DispatchComputeShader(ComputeShaderHandle handle, int32_t x, int32_t y, int32_t z) {
	ComputeShaderOpenGL computeShaderEntry = GetComputeShaderEntry(handle);
	glUseProgram(computeShaderEntry.id);
	glDispatchCompute(x, y, z);
	glUseProgram(0);
}

void RendererOpenGL::BindTexture(ComputeShaderHandle computeShaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index) {
	TextureOpenGL textureEntry = GetTextureEntry(textureHandle);
	glBindImageTexture(static_cast<GLuint>(index), textureEntry.id, 0, GL_FALSE, 0, GL_READ_WRITE, textureEntry.internalFormat);
	//SetUniform1i(computeShaderHandle, name, static_cast<int32_t>(index));
	(void)name;
	(void)computeShaderHandle;
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

ShaderOpenGL& RendererOpenGL::GetShaderEntry(ShaderHandle handle) {
	return m_shaders[handle.id];
}

ComputeShaderOpenGL& RendererOpenGL::GetComputeShaderEntry(ComputeShaderHandle handle) {
	return m_computeShaders[handle.id];
}

ShaderStorageBufferOpenGL& RendererOpenGL::GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle) {
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
