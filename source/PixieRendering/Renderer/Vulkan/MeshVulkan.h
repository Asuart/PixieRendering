#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct MeshVulkan {
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
	uint32_t indicesCount = 0;
};

} // namespace PixieRenderer
