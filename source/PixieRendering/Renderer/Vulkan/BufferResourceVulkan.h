#pragma once

#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct BufferResourceVulkan {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	void* bufferMapped = nullptr;
	uint32_t size = 0;
};

} // namespace PixieRenderer
