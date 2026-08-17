#pragma once
#include <vector>

#include "../IRenderer.h"
#include "FrameBufferOpenGL.h"
#include "MeshOpenGL.h"
#include "ShaderCompilationOpenGL.h"
#include "ShaderStorageBufferOpenGL.h"
#include "TextureOpenGL.h"
#include "UniformBufferOpenGL.h"
#include "ViewportStateOpenGL.h"

namespace PixieRenderer {

class Window;

class RendererOpenGL : public IRenderer {
  public:
	RendererOpenGL(Window* mainWindow);
	~RendererOpenGL();

	void StartFrame() override;
	void EndFrame() override;

	MeshHandle CreateMesh(const Mesh* mesh) override;
	void DestroyMesh(MeshHandle handle) override;
	void LoadMesh(MeshHandle handle, const Mesh* mesh) override;
	void DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) override;

	FrameBufferHandle CreateFrameBuffer(glm::ivec2 resolution) override;
	void DestroyFrameBuffer(FrameBufferHandle handle) override;
	void ResizeFrameBuffer(FrameBufferHandle handle, glm::ivec2 resolution) override;
	void BindFrameBuffer(FrameBufferHandle handle) override;
	void UnbindFrameBuffer() override;

	TextureHandle CreateTexture(const Image2D* image) override;
	void DestroyTexture(TextureHandle handle) override;
	void LoadTexture(TextureHandle handle, const Image2D* image) override;
	void SetTextureFiltering(
	    TextureHandle handle,
	    TextureFiltering minFilter,
	    TextureFiltering magFilter
	) override;
	void SetTextureWrap(
	    TextureHandle handle,
	    TextureWrap wrapU,
	    TextureWrap wrapV,
	    TextureWrap wrapW
	) override;
	void GenerateTextureMipmaps(TextureHandle handle) override;
	glm::ivec2 GetTextureResolution(TextureHandle handle) override;
	void BindTexture(
	    MaterialHandle materialHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint64_t index
	) override;
	void BindTexture(
	    ComputeProgramHandle computeProgramHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint64_t index
	) override;

	ShaderStorageBufferHandle
	CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) override;
	void DestroyShaderStorageBuffer(ShaderStorageBufferHandle handle) override;
	void LoadShaderStorageBuffer(
	    ShaderStorageBufferHandle handle,
	    const uint8_t* data,
	    uint32_t size
	) override;
	uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) override;
	std::vector<uint8_t> GetShaderStorageBufferData(
	    ShaderStorageBufferHandle handle,
	    uint32_t offset,
	    uint32_t size
	) override;

	UniformBufferHandle CreateUniformBuffer(const uint8_t* data, uint32_t size) override;
	void DestroyUniformBuffer(UniformBufferHandle handle) override;
	void LoadUniformBuffer(UniformBufferHandle handle, const uint8_t* data, uint32_t size) override;
	void LoadUniformBuffer(
	    MaterialHandle handle,
	    const std::string& name,
	    const void* data,
	    size_t size
	) override;

	MaterialHandle CreateMaterial(const Material* materialInfo) override;
	void DestroyMaterial(MaterialHandle handle) override;

	ComputeProgramHandle CreateComputeProgram(const char* source) override;
	void DestroyComputeProgram(ComputeProgramHandle handle) override;
	void
	DispatchComputeProgram(ComputeProgramHandle handle, int32_t x, int32_t y, int32_t z) override;

	void SetViewport(glm::ivec2 start, glm::ivec2 resolution) override;

	void WaitIdle() override;
	void MemoryBarriersAll() override;

	uint64_t GetInternalID(TextureHandle handle) override;
	uint64_t GetInternalColorAttachmentID(FrameBufferHandle handle) override;
	uint64_t GetInternalDepthAttachmentID(FrameBufferHandle handle) override;

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
	ShaderOpenGL& GetShaderEntry(MaterialHandle handle);
	ComputeShaderOpenGL& GetComputeShaderEntry(ComputeProgramHandle handle);
	ShaderStorageBufferOpenGL& GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle);
	UniformBufferOpenGL& GetUniformBufferEntry(UniformBufferHandle handle);

	void StoreViewportState();
	void RestoreViewportState();
};

} // namespace PixieRenderer
