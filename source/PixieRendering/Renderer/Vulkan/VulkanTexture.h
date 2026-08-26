#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "PixieRendering/TextureEnums.h"

namespace PixieRenderer {

class VulkanDevice;

class VulkanTexture {
  public:
	VulkanDevice& m_device;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_mipLevels = 0;
	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	VkImageView m_imageView = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
	VkFormat m_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkFilter m_minFilter = VK_FILTER_LINEAR;
	VkFilter m_magFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode m_addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode m_addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode m_addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VulkanTexture(VulkanDevice& parentDevice);
	~VulkanTexture();
};

static inline VkSamplerAddressMode ToVkSamplerAddressMode(TextureWrap wrap) {
	switch (wrap) {
	case TextureWrap::Reapeat:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case TextureWrap::MirroredRepeat:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case TextureWrap::ClampToEdge:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case TextureWrap::ClampToBorder:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	default:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static inline VkFilter ToVkFilter(TextureFiltering filtering) {
	switch (filtering) {
	case TextureFiltering::Linear:
		return VK_FILTER_LINEAR;
	case TextureFiltering::Nearest:
		return VK_FILTER_NEAREST;
	case TextureFiltering::NearestMipmapNearest:
		return VK_FILTER_NEAREST;
	case TextureFiltering::NearestMipmapLinear:
		return VK_FILTER_NEAREST;
	case TextureFiltering::LinearMipmapNearest:
		return VK_FILTER_LINEAR;
	case TextureFiltering::LinearMipmapLinear:
		return VK_FILTER_LINEAR;
	default:
		return VK_FILTER_LINEAR;
	}
}

static inline VkFormat ToVkFormat(TextureFormat format) {
	switch (format) {
	case TextureFormat::Red8:
		return VK_FORMAT_R8_SRGB;
	case TextureFormat::RGB8:
		return VK_FORMAT_R8G8B8_SRGB;
	case TextureFormat::RGBA8:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case TextureFormat::Red32f:
		return VK_FORMAT_R32_SFLOAT;
	case TextureFormat::RGB32f:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case TextureFormat::RGBA32f:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
}

} // namespace PixieRenderer
