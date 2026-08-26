#include "VulkanFrameBuffer.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanFrameBuffer::VulkanFrameBuffer(
    VulkanDevice& parentDevice,
    uint32_t width,
    uint32_t height,
    VkFormat colorFormat
)
    : m_device(parentDevice),
      m_width(width),
      m_height(height),
      m_colorFormat(colorFormat),
      m_depthFormat(m_device.FindDepthFormat()) {
	m_device.CreateImage(
	    width,
	    height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    m_colorFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    m_colorImage,
	    m_colorImageMemory
	);
	m_device.CreateImageView(
	    m_colorImage,
	    m_colorFormat,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    1,
	    m_colorImageView
	);

	m_device.CreateImage(
	    width,
	    height,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    m_depthFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    m_depthImage,
	    m_depthImageMemory
	);
	m_device.CreateImageView(
	    m_depthImage,
	    m_depthFormat,
	    VK_IMAGE_ASPECT_DEPTH_BIT,
	    1,
	    m_depthImageView
	);
	VkDevice device = m_device.GetDevice();

	m_sampler = std::make_unique<VulkanSampler>(m_device);

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = m_depthFormat;
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
	if (vkCreateRenderPass(device, &rpInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass for framebuffer");
	}

	std::array<VkImageView, 2> fbAttachments = { m_colorImageView, m_depthImageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
	fbInfo.pAttachments = fbAttachments.data();
	fbInfo.width = width;
	fbInfo.height = height;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(device, &fbInfo, nullptr, &m_framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer");
	}
}

VulkanFrameBuffer::~VulkanFrameBuffer() {
	VkDevice device = m_device.GetDevice();

	if (m_colorImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_colorImageView, nullptr);
	}
	if (m_colorImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_colorImage, nullptr);
	}
	if (m_colorImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_colorImageMemory, nullptr);
	}

	if (m_depthImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_depthImageView, nullptr);
	}
	if (m_depthImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_depthImage, nullptr);
	}
	if (m_depthImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_depthImageMemory, nullptr);
	}

	m_sampler = nullptr;

	if (m_renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, m_renderPass, nullptr);
	}

	if (m_framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, m_framebuffer, nullptr);
	}
}

void VulkanFrameBuffer::Resize(uint32_t width, uint32_t height) {
	if (fb.width == resolution.x && fb.height == resolution.y) {
		return;
	}

	VulkanTexture& colorTexture = GetTextureEntry(fb.colorTexture);
	VulkanTexture& depthTexture = GetTextureEntry(fb.depthTexture);

	if (fb.framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(m_device, fb.framebuffer, nullptr);
		fb.framebuffer = VK_NULL_HANDLE;
	}

	m_device.DestroyTexture(colorTexture);
	m_device.DestroyTexture(depthTexture);

	fb.width = resolution.x;
	fb.height = resolution.y;

	colorTexture.width = resolution.x;
	colorTexture.height = resolution.y;

	depthTexture.width = resolution.x;
	depthTexture.height = resolution.y;

	m_device.CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    colorTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    colorTexture.image,
	    colorTexture.memory
	);
	m_device.CreateImageView(
	    colorTexture.image,
	    colorTexture.format,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    1,
	    colorTexture.imageView
	);

	m_device.CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    depthTexture.image,
	    depthTexture.memory
	);
	m_device.CreateImageView(
	    depthTexture.image,
	    depthTexture.format,
	    VK_IMAGE_ASPECT_DEPTH_BIT,
	    1,
	    depthTexture.imageView
	);

	std::array<VkImageView, 2> attachments = { colorTexture.imageView, depthTexture.imageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = fb.renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbInfo.pAttachments = attachments.data();
	fbInfo.width = resolution.x;
	fbInfo.height = resolution.y;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &fb.framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to recreate framebuffer during resize!");
	}
}

} // namespace PixieRenderer
