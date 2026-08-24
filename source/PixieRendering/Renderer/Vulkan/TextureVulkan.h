#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "PixieRendering/TextureEnums.h"

namespace PixieRenderer {

struct TextureVulkan {
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t mipLevels = 0;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkFilter minFilter = VK_FILTER_LINEAR;
	VkFilter magFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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
