#pragma once
#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct UniformBufferVulkan {
	VkDeviceSize size = 0;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	void* bufferMapped = nullptr;
};

} // namespace PixieRenderer
