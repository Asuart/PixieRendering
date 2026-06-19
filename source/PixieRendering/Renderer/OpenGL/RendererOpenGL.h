#pragma once
#include <vector>

#include "../Renderer.h"
#include "MeshOpenGL.h"
#include "TextureOpenGL.h"
#include "FrameBufferOpenGL.h"
#include "ShaderCompilationOpenGL.h"
#include "ViewportStateOpenGL.h"
#include "ShaderStorageBufferOpenGL.h"
#include "UniformBufferOpenGL.h"

class MainWindow;

class RendererOpenGL : public Renderer {
public:
	RendererOpenGL(MainWindow* mainWindow);
	~RendererOpenGL();

	void StartFrame();
	void EndFrame();

	void DrawMesh(MeshHandle meshHandle, ShaderHandle shaderHandle);
	MeshHandle LoadMesh(const Mesh* mesh);
	void LoadMesh(MeshHandle& handle, const Mesh* mesh);

	FrameBufferHandle CreateFrameBuffer(glm::ivec2 resolution);
	void ResizeFrameBuffer(FrameBufferHandle& handle, glm::ivec2 resolution);
	void BindFrameBuffer(FrameBufferHandle handle);
	void UnbindFrameBuffer();
	void ClearFrameBuffer();

	void ResizeViewport(glm::ivec2 resolution);

	TextureHandle CreateTexture(glm::ivec2 resolution, TextureFormat format);
	TextureHandle LoadTexture(float* data, glm::ivec2 resolution);
	TextureHandle LoadTexture(glm::vec3* data, glm::ivec2 resolution);
	TextureHandle LoadTexture(glm::vec4* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, float* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, glm::vec3* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, glm::vec4* data, glm::ivec2 resolution);
	void SetTextureFiltering(const TextureHandle handle, TextureFiltering minFilter, TextureFiltering magFilter);
	void SetTextureWrap(const TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT);
	void GenerateTextureMipmaps(const TextureHandle handle);
	void BindTexture(ShaderHandle shaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index);
	void BindTexture(ComputeShaderHandle computeShaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index);
	glm::ivec2 GetTextureResolution(TextureHandle handle);

	ShaderStorageBufferHandle LoadShaderStorageBuffer(uint8_t* data, uint32_t size);
	void LoadShaderStorageBuffer(ShaderStorageBufferHandle handle, uint8_t* data, uint32_t size);
	void BindShaderStorageBuffer(ShaderStorageBufferHandle handle, uint32_t index);
	uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle);
	std::vector<uint8_t> GetShaderStorageBufferData(ShaderStorageBufferHandle handle, uint32_t offset, uint32_t size);

	UniformBufferHandle LoadUniformBuffer(uint8_t* data, uint32_t size);
	void LoadUniformBuffer(UniformBufferHandle handle, uint8_t* data, uint32_t size);
	void BindUniformBuffer(UniformBufferHandle handle, uint32_t index);

	ShaderHandle CreateShader(const std::string& vertexShaderSource, const std::string fragmentShaderShource);
	ComputeShaderHandle CreateComputeShader(const std::string& source);
	void DispatchComputeShader(ComputeShaderHandle handle, int32_t x, int32_t y, int32_t z);

	void MemoryBarriersAll();

	uint64_t GetInternalID(TextureHandle handle);
	uint64_t GetInternalColorAttachmentID(FrameBufferHandle handle);
	uint64_t GetInternalDepthAttachmentID(FrameBufferHandle handle);

private:
	std::vector<TextureOpenGL> m_textures;
	std::vector<MeshOpenGL> m_meshes;
	std::vector<FrameBufferOpenGL> m_frameBuffers;
	std::vector<ShaderOpenGL> m_shaders;
	std::vector<ComputeShaderOpenGL> m_computeShaders;
	std::vector<ViewportStateOpenGL> m_viewportStates;
	std::vector<ShaderStorageBufferOpenGL> m_shaderStorageBuffers;
	std::vector<UniformBufferOpenGL> m_uniformBuffers;

	TextureOpenGL& GetTextureEntry(TextureHandle handle);
	MeshOpenGL& GetMeshEntry(MeshHandle handle);
	FrameBufferOpenGL& GetFrameBufferEntry(FrameBufferHandle handle);
	ShaderOpenGL& GetShaderEntry(ShaderHandle handle);
	ComputeShaderOpenGL& GetComputeShaderEntry(ComputeShaderHandle handle);
	ShaderStorageBufferOpenGL& GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle);
	UniformBufferOpenGL& GetUniformBufferEntry(UniformBufferHandle handle);

	void StoreViewportState();
	void RestoreViewportState();
};
