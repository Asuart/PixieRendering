#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "VulkanSampler.h"

namespace PixieRenderer {

class VulkanDevice;

class VulkanFrameBuffer {
  public:
	VulkanFrameBuffer(
	    VulkanDevice& parentDevice,
	    uint32_t width,
	    uint32_t height,
	    VkFormat colorFormat
	);
	~VulkanFrameBuffer();

	VkFramebuffer GetFrameBuffer() const;
	VkRenderPass GetRenderPass() const;
	VkSampler GetSampler() const;
	VkImageView GetColorImageView() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

	void Resize(uint32_t width, uint32_t height);

  private:
	VulkanDevice& m_device;
	VkImage m_colorImage = VK_NULL_HANDLE;
	VkDeviceMemory m_colorImageMemory = VK_NULL_HANDLE;
	VkImageView m_colorImageView = VK_NULL_HANDLE;
	VkImage m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
	VkImageView m_depthImageView = VK_NULL_HANDLE;
	std::unique_ptr<VulkanSampler> m_sampler = nullptr;
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
	VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

	void CreateImages();
	void FreeImages();
};

} // namespace PixieRenderer
