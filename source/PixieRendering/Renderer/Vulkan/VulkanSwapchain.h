#pragma once
#include <vector>

#include <vulkan/vulkan.h>

namespace PixieRenderer {

class VulkanDevice;

class VulkanSwapchain {
  public:
	VulkanSwapchain(VulkanDevice& parentDevice, VkExtent2D extent, VkRenderPass renderPass);
	~VulkanSwapchain();

	VkSwapchainKHR GetSwapChain() const;

  private:
	VulkanDevice& m_device;
	VkExtent2D m_extent = { 0, 0 };
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkFormat m_format = VK_FORMAT_UNDEFINED;
	std::vector<VkImage> m_images = {};
	std::vector<VkImageView> m_imageViews = {};
	std::vector<VkFramebuffer> m_framebuffers = {};

	VkImage m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
	VkImageView m_depthImageView = VK_NULL_HANDLE;

  public:
	static VkSurfaceFormatKHR
	ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR
	ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace PixieRenderer
