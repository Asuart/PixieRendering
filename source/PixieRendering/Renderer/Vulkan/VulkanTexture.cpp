#include "VulkanTexture.h"

#include <stdexcept>

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanSampler.h"

namespace PixieRenderer {

VulkanTexture::VulkanTexture(
    VulkanDevice& parentDevice,
    uint32_t width,
    uint32_t height,
    VkFormat format
)
    : m_device(parentDevice), m_width(width), m_height(height), m_format(format) {
}

VulkanTexture::~VulkanTexture() {
	FreeVkResources();
}

void VulkanTexture::Load(uint32_t width, uint32_t height, const void* data, VkFormat format) {
	FreeVkResources();

	m_format = format;
	m_width = width;
	m_height = height;
	m_format = format;

	uint32_t maxMipLevels =
	    static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	m_mipLevels = maxMipLevels;

	VkDeviceSize imageSize = width * height * 4;
	VulkanBuffer stagingBuffer(
	    m_device,
	    imageSize,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	stagingBuffer.Load(imageSize, data);

	m_device.CreateImage(
	    m_width,
	    m_height,
	    m_mipLevels,
	    VK_SAMPLE_COUNT_1_BIT,
	    m_format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	        VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    m_image,
	    m_memory
	);

	m_device.TransitionImageLayout(
	    m_image,
	    m_format,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    m_mipLevels
	);

	m_device.CopyBufferToImage(stagingBuffer.GetBuffer(), m_image, m_width, m_height);

	GenerateMipmaps(m_mipLevels);

	m_device
	    .CreateImageView(m_image, m_format, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, m_imageView);
}

uint32_t VulkanTexture::GetWidth() const {
	return m_width;
}

uint32_t VulkanTexture::GetHeight() const {
	return m_height;
}

VkImageView VulkanTexture::GetImageView() const {
	return m_imageView;
}

VkSampler VulkanTexture::GetSampler() const {
	return m_sampler != nullptr ? m_sampler->GetSampler() : VK_NULL_HANDLE;
}

void VulkanTexture::SetSampler(const std::shared_ptr<VulkanSampler>& sampler) {
	m_sampler = sampler;
}

void VulkanTexture::SetWrap(
    VkSamplerAddressMode wrapU,
    VkSamplerAddressMode wrapV,
    VkSamplerAddressMode wrapW
) {
	if (m_sampler == nullptr) {
		return;
	}
	if (m_sampler.use_count() > 1) {
		m_sampler = std::make_shared<VulkanSampler>(*m_sampler);
	}
	m_sampler->SetWrap(wrapU, wrapV, wrapW);
}

void VulkanTexture::SetFiltering(VkFilter minFilter, VkFilter magFilter) {
	if (m_sampler == nullptr) {
		return;
	}
	if (m_sampler.use_count() > 1) {
		m_sampler = std::make_shared<VulkanSampler>(*m_sampler);
	}
	m_sampler->SetFiltering(minFilter, magFilter);
}

void VulkanTexture::GenerateMipmaps(uint32_t mipLevels) {
	if (mipLevels == m_mipLevels) {
		return;
	}

	VkPhysicalDevice physicalDevice = m_device.GetPhysicalDevice();
	VkDevice device = m_device.GetDevice();

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, m_format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
	    )) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) ||
	    !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
		throw std::runtime_error("Texture image format does not support blit operations!");
	}

	VkCommandBuffer commandBuffer = m_device.BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = m_image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = m_width;
	int32_t mipHeight = m_height;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
		    commandBuffer,
		    VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_TRANSFER_BIT,
		    0,
		    0,
		    nullptr,
		    0,
		    nullptr,
		    1,
		    &barrier
		);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
			                   mipHeight > 1 ? mipHeight / 2 : 1,
			                   1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
		    commandBuffer,
		    m_image,
		    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    m_image,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    1,
		    &blit,
		    VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
		    commandBuffer,
		    VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		    0,
		    0,
		    nullptr,
		    0,
		    nullptr,
		    1,
		    &barrier
		);

		if (mipWidth > 1) {
			mipWidth /= 2;
		}
		if (mipHeight > 1) {
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
	    commandBuffer,
	    VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    0,
	    0,
	    nullptr,
	    0,
	    nullptr,
	    1,
	    &barrier
	);

	m_mipLevels = mipLevels;

	m_device.EndSingleTimeCommands(commandBuffer);
}

void VulkanTexture::FreeVkResources() {
	VkDevice device = m_device.GetDevice();
	if (m_imageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_imageView, nullptr);
		m_imageView = VK_NULL_HANDLE;
	}
	if (m_image != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}
	if (m_memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}

} // namespace PixieRenderer
