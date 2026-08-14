#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct MeshVulkan {
	VkBuffer positionsBuffer = VK_NULL_HANDLE;
	VkDeviceMemory positionsBufferMemory = VK_NULL_HANDLE;
	VkBuffer normalsBuffer = VK_NULL_HANDLE;
	VkDeviceMemory normalsBufferMemory = VK_NULL_HANDLE;
	VkBuffer uvsBuffer = VK_NULL_HANDLE;
	VkDeviceMemory uvsBufferMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
	uint32_t indicesCount = 0;
};

} // namespace PixieRenderer
