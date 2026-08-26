#include "VulkanTexture.h"

namespace PixieRenderer {

VulkanTexture::VulkanTexture(VulkanDevice& parentDevice) : m_device(parentDevice) {
}

VulkanTexture ::~VulkanTexture() {
	if (texture.sampler != VK_NULL_HANDLE) {
		vkDestroySampler(m_device, texture.sampler, nullptr);
		texture.sampler = VK_NULL_HANDLE;
	}
	if (texture.imageView != VK_NULL_HANDLE) {
		vkDestroyImageView(m_device, texture.imageView, nullptr);
		texture.imageView = VK_NULL_HANDLE;
	}
	if (texture.image != VK_NULL_HANDLE) {
		vkDestroyImage(m_device, texture.image, nullptr);
		texture.image = VK_NULL_HANDLE;
	}
	if (texture.memory != VK_NULL_HANDLE) {
		vkFreeMemory(m_device, texture.memory, nullptr);
		texture.memory = VK_NULL_HANDLE;
	}
}

} // namespace PixieRenderer