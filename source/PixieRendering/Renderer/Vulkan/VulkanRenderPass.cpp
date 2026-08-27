#include "VulkanRenderPass.h"

namespace PixieRenderer {

VulkanRenderPass::VulkanRenderPass(VulkanDevice& parentDevice) : m_device(parentDevice) {
}

VulkanRenderPass::~VulkanRenderPass() {
}

} // namespace PixieRenderer
