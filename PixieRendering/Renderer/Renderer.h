#pragma once
#include <string>

#include "PixieRendering/Resources/Mesh.h"
#include "PixieRendering/RenderAPI.h"
#include "PixieRendering/TextureEnums.h"
#include "PixieRendering/ResourceHandles.h"

class MainWindow;

class Renderer {
public:
	Renderer(MainWindow* mainWindow, RenderAPI renderAPI) : 
		m_mainWindow(mainWindow), m_renderAPI(renderAPI) {}

	virtual ~Renderer() {};

	MainWindow* GetMainWindow() const { return m_mainWindow; }
	RenderAPI GetRenderAPI() const { return m_renderAPI; }

	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void DrawMesh(MeshHandle meshHandle, ShaderHandle shaderHandle) = 0;
	virtual MeshHandle LoadMesh(const Mesh* mesh) = 0;
	virtual void LoadMesh(MeshHandle& handle, const Mesh* mesh) = 0;

	virtual FrameBufferHandle CreateFrameBuffer(glm::ivec2 resolution) = 0;
	virtual void ResizeFrameBuffer(FrameBufferHandle& handle, glm::ivec2 resolution) = 0;
	virtual void BindFrameBuffer(FrameBufferHandle handle) = 0;
	virtual void UnbindFrameBuffer() = 0;
	virtual void ClearFrameBuffer() = 0;

	virtual void ResizeViewport(glm::ivec2 resolution) = 0;

	virtual TextureHandle CreateTexture(glm::ivec2 resolution, TextureFormat format) = 0;
	virtual TextureHandle LoadTexture(float* data, glm::ivec2 resolution) = 0;
	virtual TextureHandle LoadTexture(glm::vec3* data, glm::ivec2 resolution) = 0;
	virtual TextureHandle LoadTexture(glm::vec4* data, glm::ivec2 resolution) = 0;
	virtual void LoadTexture(TextureHandle handle, float* data, glm::ivec2 resolution) = 0;
	virtual void LoadTexture(TextureHandle handle, glm::vec3* data, glm::ivec2 resolution) = 0;
	virtual void LoadTexture(TextureHandle handle, glm::vec4* data, glm::ivec2 resolution) = 0;
	virtual void SetTextureFiltering(const TextureHandle handle, TextureFiltering minFilter, TextureFiltering magFilter) = 0;
	virtual void SetTextureWrap(const TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) = 0;
	virtual void GenerateTextureMipmaps(const TextureHandle handle) = 0;
	virtual glm::ivec2 GetTextureResolution(TextureHandle handle) = 0;
	virtual void BindTexture(ShaderHandle shaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index) = 0;
	virtual void BindTexture(ComputeShaderHandle computeShaderHandle, const std::string& name, TextureHandle textureHandle, uint64_t index) = 0;

	virtual ShaderStorageBufferHandle LoadShaderStorageBuffer(uint8_t* data, uint32_t size) = 0;
	virtual void LoadShaderStorageBuffer(ShaderStorageBufferHandle handle, uint8_t* data, uint32_t size) = 0;
	virtual void BindShaderStorageBuffer(ShaderStorageBufferHandle handle, uint32_t index) = 0;
	virtual uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) = 0;
	virtual std::vector<uint8_t> GetShaderStorageBufferData(ShaderStorageBufferHandle handle, uint32_t offset, uint32_t size) = 0;

	virtual UniformBufferHandle LoadUniformBuffer(uint8_t* data, uint32_t size) = 0;
	virtual void LoadUniformBuffer(UniformBufferHandle handle, uint8_t* data, uint32_t size) = 0;
	virtual void BindUniformBuffer(UniformBufferHandle handle, uint32_t index) = 0;

	virtual ShaderHandle CreateShader(const std::string& vertexShaderSource, const std::string fragmentShaderShource) = 0;
	virtual ComputeShaderHandle CreateComputeShader(const std::string& source) = 0;
	virtual void DispatchComputeShader(ComputeShaderHandle handle, int32_t x, int32_t y, int32_t z) = 0;

	virtual void MemoryBarriersAll() = 0;

	virtual uint64_t GetInternalID(TextureHandle handle) = 0;
	virtual uint64_t GetInternalColorAttachmentID(FrameBufferHandle handle) = 0;
	virtual uint64_t GetInternalDepthAttachmentID(FrameBufferHandle handle) = 0;

protected:
	MainWindow* m_mainWindow;
	RenderAPI m_renderAPI;
};
