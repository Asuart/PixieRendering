#pragma once
#include "../IRenderer.h"

#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "ComputeProgramVulkan.h"
#include "FrameBufferVulkan.h"
#include "MaterialVulkan.h"
#include "MeshVulkan.h"
#include "QueueFamilyIndices.h"
#include "ShaderStorageBufferVulkan.h"
#include "SwapChainSupportDetails.h"
#include "TextureVulkan.h"
#include "UniformBufferVulkan.h"

namespace PixieRenderer {

class RendererVulkan : public IRenderer {
  public: // API
	RendererVulkan(Window* window);

	void StartFrame();
	void EndFrame();

	MeshHandle CreateMesh(const Mesh* mesh = nullptr);
	void DestroyMesh(MeshHandle handle);
	void LoadMesh(MeshHandle handle, const Mesh* mesh);
	void DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle);

	FrameBufferHandle CreateFrameBuffer(glm::ivec2 resolution);
	void DestroyFrameBuffer(FrameBufferHandle handle);
	void ResizeFrameBuffer(FrameBufferHandle handle, glm::ivec2 resolution);
	void BindFrameBuffer(FrameBufferHandle handle);
	void UnbindFrameBuffer();

	TextureHandle CreateTexture(const Image2D* image);
	void DestroyTexture(TextureHandle handle);
	void LoadTexture(TextureHandle handle, const Image2D* image);
	void SetTextureFiltering(
	    TextureHandle handle,
	    TextureFiltering minFilter,
	    TextureFiltering magFilter
	);
	void SetTextureWrap(TextureHandle handle, TextureWrap wrapU, TextureWrap wrapV, TextureWrap wrapW);
	void GenerateTextureMipmaps(TextureHandle handle);
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
	void DestroyShaderStorageBuffer(ShaderStorageBufferHandle handle);
	void
	LoadShaderStorageBuffer(ShaderStorageBufferHandle handle, const uint8_t* data, uint32_t size);
	uint32_t GetShaderStorageBufferSize(ShaderStorageBufferHandle handle);
	std::vector<uint8_t>
	GetShaderStorageBufferData(ShaderStorageBufferHandle handle, uint32_t offset, uint32_t size);

	UniformBufferHandle CreateUniformBuffer(const uint8_t* data, uint32_t size);
	void DestroyUniformBuffer(UniformBufferHandle handle);
	void LoadUniformBuffer(UniformBufferHandle handle, const uint8_t* data, uint32_t size);
	void LoadUniformBuffer(
	    MaterialHandle handle,
	    const std::string& name,
	    const void* data,
	    size_t size
	);

	MaterialHandle CreateMaterial(const Material* material);
	void DestroyMaterial(MaterialHandle material);

	ComputeProgramHandle CreateComputeProgram(const char* source);
	void DestroyComputeProgram(ComputeProgramHandle handle);
	void DispatchComputeProgram(ComputeProgramHandle handle, int32_t x, int32_t y, int32_t z);

	void SetViewport(glm::ivec2 start, glm::ivec2 resolution);

	void WaitIdle();
	void MemoryBarriersAll();

	uint64_t GetInternalID(TextureHandle handle);
	uint64_t GetInternalColorAttachmentID(FrameBufferHandle handle);
	uint64_t GetInternalDepthAttachmentID(FrameBufferHandle handle);

  private: // API
	// Resources
	std::vector<MeshVulkan> m_meshes = {};
	std::vector<TextureVulkan> m_textures = {};
	std::vector<MaterialVulkan> m_materials = {};
	std::vector<ComputeProgramVulkan> m_computePrograms = {};
	std::vector<UniformBufferVulkan> m_uniformBuffers = {};
	std::vector<ShaderStorageBufferVulkan> m_shaderStorageBuffers = {};
	std::vector<FrameBufferVulkan> m_frameBuffers = {};
	// Frame render order
	struct RenderRequest {
		MeshHandle meshHandle;
		MaterialHandle materialHandle;
	};
	std::vector<RenderRequest> m_renderRequests = {};
	FrameBufferHandle m_activeFrameBuffer = FrameBufferHandle();

	TextureVulkan& GetTextureEntry(TextureHandle handle);
	MeshVulkan& GetMeshEntry(MeshHandle handle);
	MaterialVulkan& GetMaterialEntry(MaterialHandle handle);
	ComputeProgramVulkan& GetComputeProgramEntry(ComputeProgramHandle handle);
	UniformBufferVulkan& GetUniformBufferEntry(UniformBufferHandle handle);
	ShaderStorageBufferVulkan& GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle);
	FrameBufferVulkan& GetFrameBufferEntry(FrameBufferHandle handle);

  public: // BACKEND
	static constexpr uint32_t cMaxFramesInFlight = 3;

  private: // BACKEND
	// General
	bool m_framebufferResized = false;
	std::vector<const char*> m_requiredExtensions = {};
	// Instance
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	// Surface
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	// Devices
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice m_device = VK_NULL_HANDLE;
	// Queues
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	// Swapchain
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImage> m_swapChainImages = {};
	std::vector<VkImageView> m_swapChainImageViews = {};
	std::vector<VkFramebuffer> m_swapChainFramebuffers = {};
	// Render pass
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkImage m_colorImage = VK_NULL_HANDLE;
	VkDeviceMemory m_colorImageMemory = VK_NULL_HANDLE;
	VkImageView m_colorImageView = VK_NULL_HANDLE;
	VkImage m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
	VkImageView m_depthImageView = VK_NULL_HANDLE;
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

	void CleanupSwapChain();
	void RecreateSwapChain();

	void CreateInstance();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateSwapchainImageViews();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateColorResources();
	void CreateDepthResources();
	void CreateSyncObjects();
	void CreateCommandBuffers();

	void CreateImage(
	    uint32_t width,
	    uint32_t height,
	    uint32_t mipLevels,
	    VkSampleCountFlagBits numSamples,
	    VkFormat format,
	    VkImageTiling tiling,
	    VkImageUsageFlags usage,
	    VkMemoryPropertyFlags properties,
	    VkImage& image,
	    VkDeviceMemory& imageMemory
	);
	VkImageView CreateImageView(
	    VkImage image,
	    VkFormat format,
	    VkImageAspectFlags aspectFlags,
	    uint32_t mipLevels
	);
	void TransitionImageLayout(
	    VkImage image,
	    VkFormat format,
	    VkImageLayout oldLayout,
	    VkImageLayout newLayout,
	    uint32_t mipLevels
	);
	void GenerateMipmaps(
	    VkImage image,
	    VkFormat imageFormat,
	    int32_t texWidth,
	    int32_t texHeight,
	    uint32_t mipLevels
	);

	void CreateBuffer(
	    VkDeviceSize size,
	    VkBufferUsageFlags usage,
	    VkMemoryPropertyFlags properties,
	    VkBuffer& buffer,
	    VkDeviceMemory& bufferMemory
	);
	void LoadBuffer(VkBuffer dstBuffer, VkDeviceSize bufferSize, const void* bufferData);
	void FreeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

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

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat FindSupportedFormat(
	    const std::vector<VkFormat>& candidates,
	    VkImageTiling tiling,
	    VkFormatFeatureFlags features
	);
	VkFormat FindDepthFormat();
	VkSampleCountFlagBits GetMaxUsableSampleCount();
	VkSurfaceFormatKHR
	ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR
	ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	VkImageAspectFlags GetAspectMask(VkFormat format);

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool CheckValidationLayerSupport();
	bool HasStencilComponent(VkFormat format);

	static std::vector<VkVertexInputBindingDescription> GetMeshBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetMeshAttributeDescriptions();
};

} // namespace PixieRenderer
