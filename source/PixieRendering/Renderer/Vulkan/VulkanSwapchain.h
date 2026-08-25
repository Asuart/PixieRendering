#pragma once
#include <vector>

#include <vulkan/vulkan.h>

namespace PixieRenderer {

class VulkanSwapchain {
  public:
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkExtent2D m_extent = { 0, 0 };
	VkFormat m_format = VK_FORMAT_UNDEFINED;
	std::vector<VkImage> m_images = {};
	std::vector<VkImageView> m_imageViews = {};
	std::vector<VkFramebuffer> m_framebuffers = {};

	VkImage m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
	VkImageView m_depthImageView = VK_NULL_HANDLE;

	VulkanSwapchain(
	    VkSwapchainKHR swapchain,
	    VkExtent2D extent,
	    VkFormat format,
	    const std::vector<VkImage>& images,
	    const std::vector<VkImageView>& imageViews,
	    const std::vector<VkFramebuffer>& framebuffers,
	    VkImage depthImage,
	    VkImageView depthImageView,
	    VkDeviceMemory depthImageMemory
	);

	static VkSurfaceFormatKHR
	ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR
	ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace PixieRenderer
