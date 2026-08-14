#pragma once
#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct ShaderStorageBufferVulkan {
	VkDeviceSize size = 0;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
};

} // namespace PixieRenderer
