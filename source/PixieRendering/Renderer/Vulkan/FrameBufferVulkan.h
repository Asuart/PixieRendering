#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace PixieRenderer {

struct FrameBufferVulkan {
	VkImage colorImage = VK_NULL_HANDLE;
	VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
	VkImageView colorImageView = VK_NULL_HANDLE;
	VkImage depthImage = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	glm::ivec2 resolution = {0, 0};
};

} // namespace PixieRenderer
