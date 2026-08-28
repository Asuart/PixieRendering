#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "VulkanSampler.h"

namespace PixieRenderer {

class VulkanDevice;
class VulkanRenderPass;

class VulkanFrameBuffer {
  public:
	VulkanFrameBuffer(
	    VulkanDevice& parentDevice,
	    VkExtent2D extent,
	    VkFormat colorFormat,
	    VulkanRenderPass* renderPass
	);
	~VulkanFrameBuffer();

	VkFramebuffer GetFrameBuffer() const;
	VkRenderPass GetRenderPass() const;
	VkSampler GetSampler() const;
	VkImageView GetColorImageView() const;
	VkExtent2D GetExtent() const;
	VulkanRenderPass* GetRenderPassObject();

	VkViewport GetViewport() const;
	void SetViewport(VkViewport viewport);
	void ResetViewport();

	VkRect2D GetScissor() const;
	void SetScissor(VkRect2D scissor);
	void ResetScissor();

	void Resize(VkExtent2D extent);

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
	VulkanRenderPass* m_renderPass = nullptr;
	VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
	VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_extent = { 0, 0 };
	VkViewport m_viewport = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	VkRect2D m_scissor = { { 0, 0 }, { 0, 0 } };

	void CreateImages();
	void FreeImages();
};

} // namespace PixieRenderer
