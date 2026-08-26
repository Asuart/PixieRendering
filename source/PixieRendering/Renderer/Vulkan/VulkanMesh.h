#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "../../Resources/Mesh.h"

namespace PixieRenderer {

class VulkanDevice;

class VulkanMesh {
  public:
	VulkanMesh(VulkanDevice& parentDevice);
	~VulkanMesh();

	void Load(const Mesh* mesh);

  private:
	VulkanDevice& m_device;
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
	uint32_t m_indicesCount = 0;
};

} // namespace PixieRenderer
