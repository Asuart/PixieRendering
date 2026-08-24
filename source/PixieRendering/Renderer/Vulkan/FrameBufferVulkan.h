#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct FrameBufferVulkan {
	TextureHandle colorTexture = {};
	TextureHandle depthTexture = {};
	//VkImage colorImage = VK_NULL_HANDLE;
	//VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
	//VkImageView colorImageView = VK_NULL_HANDLE;
	//VkImage depthImage = VK_NULL_HANDLE;
	//VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
	//VkImageView depthImageView = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0;
};

} // namespace PixieRenderer
