#include "VulkanDevice.h"

#include <array>
#include <set>
#include <stdexcept>
#include <vector>

#include "VulkanSwapchain.h"

namespace PixieRenderer {

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

void VulkanDevice::Initialize(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	m_physicalDevice = physicalDevice;
	m_surface = surface;
	m_queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);
	CreateLogicalDevice();
	CreateCommandPool(m_commandPool);
}

void VulkanDevice::Cleanup() {
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyDevice(m_device, nullptr);
}

std::unique_ptr<VulkanSwapchain>
VulkanDevice::CreateSwapchain(VkExtent2D extent, VkRenderPass renderPass) {
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat =
	    VulkanSwapchain::ChooseSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
	    VulkanSwapchain::ChoosePresentMode(swapChainSupport.presentModes);
	VkExtent2D realExtent = VulkanSwapchain::ChooseExtent(extent, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
	    imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = realExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { m_queueFamilyIndices.graphicsFamily,
		                              m_queueFamilyIndices.presentFamily };

	if (m_queueFamilyIndices.graphicsFamily != m_queueFamilyIndices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	std::vector<VkImage> swapChainImages = {};
	if (vkGetSwapchainImagesKHR(m_device, swapchain, &imageCount, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to retrieve swapchain image count!");
	}
	swapChainImages.resize(imageCount);
	if (vkGetSwapchainImagesKHR(m_device, swapchain, &imageCount, swapChainImages.data()) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to retrieve swapchain images!");
	}

	std::vector<VkImageView> swapChainImageViews(swapChainImages.size());
	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		CreateImageView(
		    swapChainImages[i],
		    surfaceFormat.format,
		    VK_IMAGE_ASPECT_COLOR_BIT,
		    1,
		    swapChainImageViews[i]
		);
	}

	VkFormat depthFormat = FindDepthFormat();
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;

	CreateImage(
	    realExtent.width,
	    realExtent.height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    depthImage,
	    depthImageMemory
	);
	CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, depthImageView);
	TransitionImageLayout(
	    depthImage,
	    depthFormat,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    1
	);

	std::vector<VkFramebuffer> swapChainFramebuffers(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageView };

		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbInfo.pAttachments = attachments.data();
		fbInfo.width = realExtent.width;
		fbInfo.height = realExtent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &swapChainFramebuffers[i]) !=
		    VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	return std::make_unique<VulkanSwapchain>(
	    swapchain,
	    realExtent,
	    surfaceFormat.format,
	    swapChainImages,
	    swapChainImageViews,
	    swapChainFramebuffers,
	    depthImage,
	    depthImageView,
	    depthImageMemory
	);
}

void VulkanDevice::DestroySwapchain(std::unique_ptr<VulkanSwapchain> swapchain) {
	vkDestroyImageView(m_device, swapchain->m_depthImageView, nullptr);
	vkDestroyImage(m_device, swapchain->m_depthImage, nullptr);
	vkFreeMemory(m_device, swapchain->m_depthImageMemory, nullptr);

	for (auto framebuffer : swapchain->m_framebuffers) {
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}

	for (auto imageView : swapchain->m_imageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, swapchain->m_swapchain, nullptr);
}

void VulkanDevice::CreateImage(
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    VkSampleCountFlagBits numSamples,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory
) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_device, image, imageMemory, 0);
}

void VulkanDevice::DestroyImage(VkImage image, VkDeviceMemory memory) {
	vkDestroyImage(m_device, image, nullptr);
	vkFreeMemory(m_device, memory, nullptr);
}

void VulkanDevice::CreateImageView(
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    uint32_t mipLevels,
    VkImageView& outImageView
) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_device, &viewInfo, nullptr, &outImageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}
}

void VulkanDevice::DestroyImageView(VkImageView imageView) {
	vkDestroyImageView(m_device, imageView, nullptr);
}

void VulkanDevice::CreateSampler(VkSampler& outSampler) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &outSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer color sampler!");
	}
}

void VulkanDevice::CreateSampler(const VulkanTexture& texture, VkSampler& outSampler) {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = texture.magFilter;
	samplerInfo.minFilter = texture.minFilter;
	samplerInfo.addressModeU = texture.addressModeU;
	samplerInfo.addressModeV = texture.addressModeV;
	samplerInfo.addressModeW = texture.addressModeW;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;
	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &outSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanDevice::DestroySampler(VkSampler sampler) {
	vkDestroySampler(m_device, sampler, nullptr);
}

void VulkanDevice::DestroyTexture(VulkanTexture& texture) {
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

void VulkanDevice::GenerateMipmaps(
    VkImage image,
    VkFormat imageFormat,
    int32_t texWidth,
    int32_t texHeight,
    uint32_t mipLevels
) {
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
	    )) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) ||
	    !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
		throw std::runtime_error("Texture image format does not support blit operations!");
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

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
		    image,
		    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    image,
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

	EndSingleTimeCommands(m_commandPool, commandBuffer);
}

void VulkanDevice::CreateRenderPass(VkFormat format, VkRenderPass& outRenderPass) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &outRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanDevice::DestroyRenderPass(VkRenderPass renderPass) {
	vkDestroyRenderPass(m_device, renderPass, nullptr);
}

void VulkanDevice::CreateCommandPool(VkCommandPool& outCommandPool) {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily;

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &outCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void VulkanDevice::DestroyCommandPool(VkCommandPool commandPool) {
	vkDestroyCommandPool(m_device, commandPool, nullptr);
}

void VulkanDevice::CreateCommandBuffer(
    VkCommandPool commandPool,
    VkCommandBuffer& outCommandBufer
) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_device, &allocInfo, &outCommandBufer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanDevice::DestroyCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
	vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
}

void VulkanDevice::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory
) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void VulkanDevice::FreeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	vkDestroyBuffer(m_device, buffer, nullptr);
	vkFreeMemory(m_device, bufferMemory, nullptr);
}

void VulkanDevice::LoadBuffer(VkBuffer dstBuffer, VkDeviceSize bufferSize, const void* bufferData) {
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
	    bufferSize,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	    stagingBuffer,
	    stagingBufferMemory
	);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, bufferData, (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	CopyBuffer(stagingBuffer, dstBuffer, bufferSize);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_commandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(m_commandPool, commandBuffer);
}

void VulkanDevice::CopyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height
) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(
	    commandBuffer,
	    buffer,
	    image,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    1,
	    &region
	);

	EndSingleTimeCommands(m_commandPool, commandBuffer);
}

void VulkanDevice::CreateFrameBuffer(
    uint32_t width,
    uint32_t height,
	VkFormat format,
    FrameBufferVulkan& outFrameBuffer
) {
	outFrameBuffer.width = width;
	outFrameBuffer.height = height;

	VulkanTexture colorTexture;
	colorTexture.width = width;
	colorTexture.height = height;
	colorTexture.format = format;

	VulkanTexture depthTexture;
	depthTexture.width = width;
	depthTexture.height = height;
	depthTexture.format = FindDepthFormat();

	CreateImage(
	    width,
	    height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    colorTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    colorTexture.image,
	    colorTexture.memory
	);
	CreateImageView(
	    colorTexture.image,
	    colorTexture.format,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    1,
	    colorTexture.imageView
	);
	CreateSampler(colorTexture.sampler);

	CreateImage(
	    width,
	    height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    depthTexture.image,
	    depthTexture.memory
	);
	CreateImageView(
	    depthTexture.image,
	    depthTexture.format,
	    VK_IMAGE_ASPECT_DEPTH_BIT,
	    1,
	    depthTexture.imageView
	);

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = colorTexture.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthTexture.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	rpInfo.pAttachments = attachments.data();
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;

	VkRenderPass renderPass;
	if (vkCreateRenderPass(m_device, &rpInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass for framebuffer");
	}

	std::array<VkImageView, 2> fbAttachments = { colorTexture.imageView, depthTexture.imageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
	fbInfo.pAttachments = fbAttachments.data();
	fbInfo.width = width;
	fbInfo.height = height;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &outFrameBuffer.framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer");
	}

	outFrameBuffer.renderPass = renderPass;
	outFrameBuffer.colorTexture = TextureHandle(m_textures.size());
	m_textures.push_back(colorTexture);
	outFrameBuffer.depthTexture = TextureHandle(m_textures.size());
	m_textures.push_back(depthTexture);
}

void VulkanDevice::DestroyFrameBuffer(FrameBufferVulkan& frameBuffer) {

}

void VulkanDevice::CreateFence(VkFence& outFence) {
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(m_device, &fenceInfo, nullptr, &outFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to create fence!");
	}
}

void VulkanDevice::DestroyFence(VkFence fence) {
	vkDestroyFence(m_device, fence, nullptr);
}

void VulkanDevice::ResetFences(uint32_t fenceCount, const VkFence* fences) {
	vkResetFences(m_device, fenceCount, fences);
}

void VulkanDevice::WaitFences(
    uint32_t fenceCount,
    const VkFence* fences,
    VkBool32 waitAll,
    uint64_t timeout
) {
	vkWaitForFences(m_device, fenceCount, fences, waitAll, timeout);
}

void VulkanDevice::CreateSemaphore(VkSemaphore& outSemaphore) {
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &outSemaphore);
}

void VulkanDevice::DestroySemaphore(VkSemaphore semaphore) {
	vkDestroySemaphore(m_device, semaphore, nullptr);
}

void VulkanDevice::WaitIdle() {
	vkDeviceWaitIdle(m_device);
}

VkCommandBuffer VulkanDevice::BeginSingleTimeCommands(VkCommandPool commandPool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
}

void VulkanDevice::TransitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    uint32_t mipLevels
) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = GetAspectMask(format);
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(
	    commandBuffer,
	    sourceStage,
	    destinationStage,
	    0,
	    0,
	    nullptr,
	    0,
	    nullptr,
	    1,
	    &barrier
	);

	EndSingleTimeCommands(m_commandPool, commandBuffer);
}

QueueFamilyIndices VulkanDevice::GetQueueFamilyIndices() const {
	return m_queueFamilyIndices;
}

SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport() const {
	return QuerySwapChainSupport(m_physicalDevice, m_surface);
}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
		    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat VulkanDevice::FindDepthFormat() {
	return FindSupportedFormat(
	    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat VulkanDevice::FindSupportedFormat(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR &&
		    (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
		           (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

void VulkanDevice::CreateLogicalDevice() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilyIndices.graphicsFamily,
		                                       m_queueFamilyIndices.presentFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily, 0, &m_presentQueue);
}

QueueFamilyIndices
VulkanDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
	    physicalDevice,
	    &queueFamilyCount,
	    queueFamilies.data()
	);

	for (int32_t i = 0; i < queueFamilies.size(); i++) {
		const auto& queueFamily = queueFamilies[i];

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.IsComplete()) {
			break;
		}
	}

	return indices;
}

bool VulkanDevice::CheckExtensionSupport(VkPhysicalDevice physicalDevice) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
	    physicalDevice,
	    nullptr,
	    &extensionCount,
	    availableExtensions.data()
	);

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails
VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
		    physicalDevice,
		    surface,
		    &formatCount,
		    details.formats.data()
		);
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
		    physicalDevice,
		    surface,
		    &presentModeCount,
		    details.presentModes.data()
		);
	}

	return details;
}

VkImageAspectFlags VulkanDevice::GetAspectMask(VkFormat format) {
	switch (format) {
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

} // namespace PixieRenderer
