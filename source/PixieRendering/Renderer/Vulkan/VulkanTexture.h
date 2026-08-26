#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>

#include "PixieRendering/TextureEnums.h"
#include "VulkanSampler.h"

namespace PixieRenderer {

class VulkanDevice;

class VulkanTexture {
  public:
	VulkanTexture(VulkanDevice& parentDevice, uint32_t width, uint32_t height, VkFormat format);
	~VulkanTexture();

	void Load(uint32_t width, uint32_t height, const void* data, VkFormat format);

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	VkImageView GetImageView() const;
	VkSampler GetSampler() const;

	void SetSampler(const std::shared_ptr<VulkanSampler>& sampler);
	void
	SetWrap(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV, VkSamplerAddressMode wrapW);
	void SetFiltering(VkFilter minFilter, VkFilter magFilter);
	void GenerateMipmaps(uint32_t mipLevels);

  private:
	VulkanDevice& m_device;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_mipLevels = 0;
	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	VkImageView m_imageView = VK_NULL_HANDLE;
	VkFormat m_format = VK_FORMAT_R8G8B8A8_SRGB;
	std::shared_ptr<VulkanSampler> m_sampler = nullptr;

	void FreeVkResources();
};

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
