#include "VulkanSampler.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanSampler::VulkanSampler(VulkanDevice& parentDevice) : m_device(parentDevice) {
	CreateVkSampler();
}

VulkanSampler::~VulkanSampler() {
	VkDevice device = m_device.GetDevice();
	if (m_sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, m_sampler, nullptr);
	}
}

VkSampler VulkanSampler::GetSampler() const {
	return m_sampler;
}

void VulkanSampler::SetWrap(
    VkSamplerAddressMode wrapU,
    VkSamplerAddressMode wrapV,
    VkSamplerAddressMode wrapW
) {
	VkDevice device = m_device.GetDevice();

	m_addressModeU = wrapU;
	m_addressModeV = wrapU;
	m_addressModeW = wrapW;

	if (m_sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, m_sampler, nullptr);
		m_sampler = VK_NULL_HANDLE;
	}

	CreateVkSampler();
}

void VulkanSampler::SetFiltering(VkFilter minFilter, VkFilter magFilter) {
	VkDevice device = m_device.GetDevice();

	m_minFilter = minFilter;
	m_magFilter = magFilter;

	if (m_sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, m_sampler, nullptr);
		m_sampler = VK_NULL_HANDLE;
	}

	CreateVkSampler();
}

void VulkanSampler::CreateVkSampler() {
	VkDevice device = m_device.GetDevice();

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = m_magFilter;
	samplerInfo.minFilter = m_minFilter;
	samplerInfo.addressModeU = m_addressModeU;
	samplerInfo.addressModeV = m_addressModeV;
	samplerInfo.addressModeW = m_addressModeW;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;

	if (m_anisotropyEnabled) {
		VkPhysicalDevice physicalDevice = m_device.GetPhysicalDevice();

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	} else {
		samplerInfo.anisotropyEnable = VK_FALSE;
	}

	if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

} // namespace PixieRenderer
