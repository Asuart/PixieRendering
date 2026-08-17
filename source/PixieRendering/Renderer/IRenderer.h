#pragma once
#include <string>

#include "PixieRendering/RenderAPI.h"
#include "PixieRendering/ResourceHandles.h"
#include "PixieRendering/Resources/Image2D.h"
#include "PixieRendering/Resources/Material.h"
#include "PixieRendering/Resources/Mesh.h"
#include "PixieRendering/TextureEnums.h"

namespace PixieRenderer {

class Window;

class IRenderer {
  public:
	IRenderer(Window* window, RenderAPI renderAPI) : m_window(window), m_renderAPI(renderAPI) {
	}

	virtual ~IRenderer() {};

	Window* GetWindow() const {
		return m_window;
	}

	RenderAPI GetRenderAPI() const {
		return m_renderAPI;
	}

	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;

	virtual MeshHandle CreateMesh(const Mesh* mesh) = 0;
	virtual void DestroyMesh(MeshHandle handle) = 0;
	virtual void LoadMesh(MeshHandle handle, const Mesh* mesh) = 0;
	virtual void DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) = 0;

	virtual FrameBufferHandle CreateFrameBuffer(glm::ivec2 resolution) = 0;
	virtual void DestroyFrameBuffer(FrameBufferHandle handle) = 0;
	virtual void ResizeFrameBuffer(FrameBufferHandle handle, glm::ivec2 resolution) = 0;
	virtual void BindFrameBuffer(FrameBufferHandle handle) = 0;
	virtual void UnbindFrameBuffer() = 0;

	virtual TextureHandle CreateTexture(const Image2D* image) = 0;
	virtual void DestroyTexture(TextureHandle handle) = 0;
	virtual void LoadTexture(TextureHandle handle, const Image2D* image) = 0;
	virtual void SetTextureFiltering(
	    TextureHandle handle,
	    TextureFiltering minFilter,
	    TextureFiltering magFilter
	) = 0;
	virtual void SetTextureWrap(
	    TextureHandle handle,
	    TextureWrap wrapU,
	    TextureWrap wrapV,
	    TextureWrap wrapW
	) = 0;
	virtual void GenerateTextureMipmaps(TextureHandle handle) = 0;
	virtual glm::ivec2 GetTextureResolution(TextureHandle handle) = 0;
	virtual void BindTexture(
	    MaterialHandle materialHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint64_t index
	) = 0;
	virtual void BindTexture(
	    ComputeProgramHandle computeProgramHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint64_t index
	) = 0;

	virtual ShaderStorageBufferHandle
	CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) = 0;
	virtual void DestroyShaderStorageBuffer(ShaderStorageBufferHandle handle) = 0;
	virtual void LoadShaderStorageBuffer(
	    ShaderStorageBufferHandle handle,
	    const uint8_t* data,
	    uint32_t size
	) = 0;
	virtual uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) = 0;
	virtual std::vector<uint8_t> GetShaderStorageBufferData(
	    ShaderStorageBufferHandle handle,
	    uint32_t offset,
	    uint32_t size
	) = 0;

	virtual UniformBufferHandle CreateUniformBuffer(const uint8_t* data, uint32_t size) = 0;
	virtual void DestroyUniformBuffer(UniformBufferHandle handle) = 0;
	virtual void
	LoadUniformBuffer(UniformBufferHandle handle, const uint8_t* data, uint32_t size) = 0;
	virtual void LoadUniformBuffer(
	    MaterialHandle handle,
	    const std::string& name,
	    const void* data,
	    size_t size
	) = 0;

	virtual MaterialHandle CreateMaterial(const Material* materialInfo) = 0;
	virtual void DestroyMaterial(MaterialHandle handle) = 0;

	virtual ComputeProgramHandle CreateComputeProgram(const char* source) = 0;
	virtual void DestroyComputeProgram(ComputeProgramHandle handle) = 0;
	virtual void
	DispatchComputeProgram(ComputeProgramHandle handle, int32_t x, int32_t y, int32_t z) = 0;

	virtual void SetViewport(glm::ivec2 start, glm::ivec2 resolution) = 0;

	virtual void WaitIdle() = 0;
	virtual void MemoryBarriersAll() = 0;

	virtual uint64_t GetInternalID(TextureHandle handle) = 0;
	virtual uint64_t GetInternalColorAttachmentID(FrameBufferHandle handle) = 0;
	virtual uint64_t GetInternalDepthAttachmentID(FrameBufferHandle handle) = 0;

  protected:
	Window* m_window = nullptr;
	RenderAPI m_renderAPI = RenderAPI::Undefined;
	uint32_t m_renderWidth = 0;
	uint32_t m_renderHeight = 0;
	glm::ivec2 m_viewportStart = { 0, 0 };
	glm::ivec2 m_viewportResolution = { 0, 0 };
};

} // namespace PixieRenderer
