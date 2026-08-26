#include "VulkanSwapchain.h"

#include <algorithm>

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanSwapchain::VulkanSwapchain(
    VulkanDevice& parentDevice,
    VkExtent2D extent,
    VkRenderPass renderPass
)
    : m_device(parentDevice), m_renderPass(renderPass) {
	VkDevice device = m_device.GetDevice();

	SwapChainSupportDetails swapChainSupport = m_device.QuerySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat =
	    VulkanSwapchain::ChooseSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
	    VulkanSwapchain::ChoosePresentMode(swapChainSupport.presentModes);
	m_extent = VulkanSwapchain::ChooseExtent(extent, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
	    imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_device.GetSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = m_extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_device.GetQueueFamilyIndices();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	if (vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to retrieve swapchain image count!");
	}

	m_images.resize(imageCount);
	if (vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to retrieve swapchain images!");
	}

	std::vector<VkImageView> swapChainImageViews(m_images.size());
	for (uint32_t i = 0; i < m_images.size(); i++) {
		m_device.CreateImageView(
		    m_images[i],
		    surfaceFormat.format,
		    VK_IMAGE_ASPECT_COLOR_BIT,
		    1,
		    swapChainImageViews[i]
		);
	}

	VkFormat depthFormat = m_device.FindDepthFormat();
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;

	m_device.CreateImage(
	    m_extent.width,
	    m_extent.height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    depthImage,
	    depthImageMemory
	);
	m_device.CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, depthImageView);
	m_device.TransitionImageLayout(
	    depthImage,
	    depthFormat,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    1
	);

	std::vector<VkFramebuffer> swapChainFramebuffers(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageView };

		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbInfo.pAttachments = attachments.data();
		fbInfo.width = m_extent.width;
		fbInfo.height = m_extent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(device, &fbInfo, nullptr, &swapChainFramebuffers[i]) !=
		    VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

VulkanSwapchain::~VulkanSwapchain() {
	VkDevice device = m_device.GetDevice();

	vkDestroyImageView(device, m_depthImageView, nullptr);
	vkDestroyImage(device, m_depthImage, nullptr);
	vkFreeMemory(device, m_depthImageMemory, nullptr);

	for (auto framebuffer : m_framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto imageView : m_imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, m_swapchain, nullptr);
}

VkSwapchainKHR VulkanSwapchain::GetSwapChain() const {
	return m_swapchain;
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
