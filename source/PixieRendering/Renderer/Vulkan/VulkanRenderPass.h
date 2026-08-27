#pragma once
#include <vulkan/vulkan.h>

namespace PixieRenderer {

class VulkanDevice;

class VulkanRenderPass {
  public:
	VulkanRenderPass(VulkanDevice& parentDevice);
	~VulkanRenderPass();

  private:
	VulkanDevice& m_device;
};

} // namespace PixieRenderer