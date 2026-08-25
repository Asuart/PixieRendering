#include "VulkanSwapchain.h"

#include <algorithm>

namespace PixieRenderer {

VulkanSwapchain::VulkanSwapchain(
    VkSwapchainKHR swapchain,
    VkExtent2D extent,
    VkFormat format,
    const std::vector<VkImage>& images,
    const std::vector<VkImageView>& imageViews,
    const std::vector<VkFramebuffer>& framebuffers,
    VkImage depthImage,
    VkImageView depthImageView,
    VkDeviceMemory depthImageMemory
)
    : m_swapchain(swapchain),
      m_extent(extent),
      m_format(format),
      m_images(images),
      m_imageViews(imageViews),
      m_framebuffers(framebuffers),
      m_depthImage(depthImage),
      m_depthImageView(depthImageView),
      m_depthImageMemory(depthImageMemory) {
}

VkSurfaceFormatKHR
VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
		    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>&) {
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanSwapchain::ChooseExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities) {
	VkExtent2D actualExtent{};
	actualExtent.width = std::clamp(
	    extent.width,
	    capabilities.minImageExtent.width,
	    capabilities.maxImageExtent.width
	);
	actualExtent.height = std::clamp(
	    extent.height,
	    capabilities.minImageExtent.height,
	    capabilities.maxImageExtent.height
	);
	return actualExtent;
}

} // namespace PixieRenderer
