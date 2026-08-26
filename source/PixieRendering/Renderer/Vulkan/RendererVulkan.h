#pragma once
#include "../IRenderer.h"

#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "ComputeProgramVulkan.h"
#include "VulkanFrameBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"

namespace PixieRenderer {

class RendererVulkan : public IRenderer {
  public: // API
	RendererVulkan(Window* window);

	void SetRenderResolution(uint32_t width, uint32_t height);

	void StartFrame();
	void EndFrame();

	void BeginRenderPass();
	void EndRenderPass();

	MeshHandle CreateMesh(const Mesh* mesh = nullptr);
	void LoadMesh(MeshHandle handle, const Mesh* mesh);
	void DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle);

	FrameBufferHandle CreateFrameBuffer(glm::uvec2 resolution, TextureFormat format);
	void ResizeFrameBuffer(FrameBufferHandle handle, glm::uvec2 resolution);
	void BindFrameBuffer(FrameBufferHandle handle);
	void UnbindFrameBuffer();

	TextureHandle CreateTexture(const Image2D* image);
	void LoadTexture(TextureHandle handle, const Image2D* image);
	void SetTextureFiltering(
	    TextureHandle handle,
	    TextureFiltering minFilter,
	    TextureFiltering magFilter
	);
	void
	SetTextureWrap(TextureHandle handle, TextureWrap wrapU, TextureWrap wrapV, TextureWrap wrapW);
	void GenerateTextureMipmaps(TextureHandle handle, uint32_t);
	glm::ivec2 GetTextureResolution(TextureHandle handle);
	void BindTexture(
	    MaterialHandle materialHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint32_t index
	);
	void BindTexture(
	    ComputeProgramHandle computeProgramHandle,
	    const std::string& name,
	    TextureHandle textureHandle,
	    uint32_t index
	);

	ShaderStorageBufferHandle CreateShaderStorageBuffer(const uint8_t* data, uint32_t size);
	void
	LoadShaderStorageBuffer(ShaderStorageBufferHandle handle, const uint8_t* data, uint32_t size);
	uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle);
	std::vector<uint8_t>
	GetShaderStorageBufferData(ShaderStorageBufferHandle handle, uint32_t offset, uint32_t size);

	UniformBufferHandle CreateUniformBuffer(const uint8_t* data, uint32_t size);
	void LoadUniformBuffer(UniformBufferHandle handle, const uint8_t* data, uint32_t size);
	void LoadUniformBuffer(
	    MaterialHandle handle,
	    const std::string& name,
	    const void* data,
	    size_t size
	);

	MaterialHandle CreateMaterial(const Material* material);

	ComputeProgramHandle CreateComputeProgram(const char* source);
	void DispatchComputeProgram(ComputeProgramHandle handle, int32_t x, int32_t y, int32_t z);

	void SetViewport(glm::ivec2 start, glm::ivec2 resolution);

	void WaitIdle();
	void MemoryBarriersAll();

	VkInstance GetInstance() const;
	VkPhysicalDevice GetPhysicalDevice() const;
	VkDevice GetDevice() const;
	VkQueue GetGraphicsQueue() const;
	VkQueue GetPresentQueue() const;
	VkCommandBuffer GetCommandBuffer() const;
	VkRenderPass GetRenderPass() const;

  private: // API
	// Resources
	std::vector<VulkanMesh> m_meshes = {};
	std::vector<VulkanTexture> m_textures = {};
	std::vector<VulkanMaterial> m_materials = {};
	std::vector<ComputeProgramVulkan> m_computePrograms = {};
	std::vector<VulkanBuffer> m_uniformBuffers = {};
	std::vector<VulkanBuffer> m_shaderStorageBuffers = {};
	std::vector<VulkanFrameBuffer> m_frameBuffers = {};
	// Frame render order
	struct RenderRequest {
		MeshHandle meshHandle;
		MaterialHandle materialHandle;
	};
	std::vector<RenderRequest> m_renderRequests = {};
	FrameBufferHandle m_activeFrameBuffer = FrameBufferHandle();

	VulkanTexture& GetTextureEntry(TextureHandle handle);
	VulkanMesh& GetMeshEntry(MeshHandle handle);
	VulkanMaterial& GetMaterialEntry(MaterialHandle handle);
	ComputeProgramVulkan& GetComputeProgramEntry(ComputeProgramHandle handle);
	VulkanBuffer& GetUniformBufferEntry(UniformBufferHandle handle);
	VulkanBuffer& GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle);
	VulkanFrameBuffer& GetFrameBufferEntry(FrameBufferHandle handle);

  public:
	VkImageView GetTextureImageView(TextureHandle handle);
	VkSampler GetTextureSmapler(TextureHandle handle);

  public: // BACKEND
	static constexpr uint32_t cMaxFramesInFlight = 3;

  private: // BACKEND
	VulkanDevice m_device;
	std::unique_ptr<VulkanSwapchain> m_swapchain = nullptr;

	// General
	bool m_framebufferResized = false;
	// Instance
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	// Surface
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	// Render pass
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	// Command pool
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers = {};
	// Synchronization
	std::vector<VkSemaphore> m_imageAvailableSemaphores = {};
	std::vector<VkSemaphore> m_renderFinishedSemaphores = {};
	std::vector<VkFence> m_inFlightFences = {};
	uint32_t m_currentFrame = 0;
	uint32_t m_nextImageIndex = 0;

	void InitVulkan();
	void Cleanup();

	void RecreateSwapChain();

	void CreateInstance();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void CreateSurface();
	void InitializeDevice();
	void CreateSyncObjects();

	void GenerateMipmaps(
	    VkImage image,
	    VkFormat imageFormat,
	    int32_t texWidth,
	    int32_t texHeight,
	    uint32_t mipLevels
	);

	void CreateMaterialDescriptorSetLayout(
	    const std::vector<ShaderBinding>& bindings,
	    VkDescriptorSetLayout& outDescriptorSetLayout
	);
	void CreateMaterialDescriptorPool(
	    const std::vector<ShaderBinding>& bindings,
	    VkDescriptorPool& outDescriptorPool
	);
	void CreateMaterialPipelineLayout(
	    VkDescriptorSetLayout descriptorSetLayout,
	    VkPipelineLayout& outPipelineLayout
	);
	void CreateMaterialPipeline(
	    VkPipelineLayout pipelineLayout,
	    VkRenderPass renderPass,
	    const VkPipelineShaderStageCreateInfo* shaderStages,
	    uint32_t shaderStagesCount,
	    VkPipeline& outPipeline
	);

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckValidationLayerSupport();

	static std::vector<VkVertexInputBindingDescription> GetMeshBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetMeshAttributeDescriptions();
};

} // namespace PixieRenderer
