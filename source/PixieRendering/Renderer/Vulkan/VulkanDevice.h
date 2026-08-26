#pragma once
#include <memory>

#include <vulkan/vulkan.h>

#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"
#include "VulkanTexture.h"
#include "FramebufferVulkan.h"

namespace PixieRenderer {

class VulkanSwapchain;

class VulkanDevice {
  public:
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	QueueFamilyIndices m_queueFamilyIndices = {};
	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	void Initialize(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void Cleanup();

	std::unique_ptr<VulkanSwapchain> CreateSwapchain(VkExtent2D extent, VkRenderPass renderPass);
	void DestroySwapchain(std::unique_ptr<VulkanSwapchain> swapchain);

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
	void DestroyImage(VkImage image, VkDeviceMemory memory);
	void CreateImageView(
	    VkImage image,
	    VkFormat format,
	    VkImageAspectFlags aspectFlags,
	    uint32_t mipLevels,
	    VkImageView& outImageView
	);
	void DestroyImageView(VkImageView imageView);
	void CreateSampler(VkSampler& outSampler);
	void CreateSampler(const VulkanTexture& texture, VkSampler& outSampler);
	void DestroySampler(VkSampler sampler);
	void DestroyTexture(VulkanTexture& texture);
	void GenerateMipmaps(
	    VkImage image,
	    VkFormat imageFormat,
	    int32_t texWidth,
	    int32_t texHeight,
	    uint32_t mipLevels
	);

	void CreateRenderPass(VkFormat format, VkRenderPass& outRenderPass);
	void DestroyRenderPass(VkRenderPass renderPass);

	void CreateCommandPool(VkCommandPool& outCommandPool);
	void DestroyCommandPool(VkCommandPool commandPool);

	void CreateCommandBuffer(VkCommandPool commandPool, VkCommandBuffer& outCommandBuffer);
	void DestroyCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	void CreateBuffer(
	    VkDeviceSize size,
	    VkBufferUsageFlags usage,
	    VkMemoryPropertyFlags properties,
	    VkBuffer& buffer,
	    VkDeviceMemory& bufferMemory
	);
	void FreeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void LoadBuffer(VkBuffer dstBuffer, VkDeviceSize bufferSize, const void* bufferData);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void CreateFrameBuffer(
	    uint32_t width,
	    uint32_t height,
	    VkFormat format,
	    FrameBufferVulkan& outFrameBuffer
	);
	void DestroyFrameBuffer(FrameBufferVulkan& frameBuffer);

	void CreateFence(VkFence& outFence);
	void DestroyFence(VkFence fence);
	void ResetFences(uint32_t fenceCount, const VkFence* fences);
	void WaitFences(uint32_t fenceCount, const VkFence* fences, VkBool32 waitAll, uint64_t timeout);

	void CreateSemaphore(VkSemaphore& outSemaphore);
	void DestroySemaphore(VkSemaphore semaphore);

	void WaitIdle();

	VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool);
	void EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	void TransitionImageLayout(
	    VkImage image,
	    VkFormat format,
	    VkImageLayout oldLayout,
	    VkImageLayout newLayout,
	    uint32_t mipLevels
	);

	QueueFamilyIndices GetQueueFamilyIndices() const;
	SwapChainSupportDetails QuerySwapChainSupport() const;

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(
	    const std::vector<VkFormat>& candidates,
	    VkImageTiling tiling,
	    VkFormatFeatureFlags features
	);

  private:
	void CreateLogicalDevice();

  public:
	static QueueFamilyIndices
	FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	static bool CheckExtensionSupport(VkPhysicalDevice physicalDevice);
	static SwapChainSupportDetails
	QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	static VkImageAspectFlags GetAspectMask(VkFormat format);
};

} // namespace PixieRenderer
