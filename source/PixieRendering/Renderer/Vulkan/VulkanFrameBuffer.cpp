#include "VulkanFrameBuffer.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanFrameBuffer::VulkanFrameBuffer(
    VulkanDevice& parentDevice,
    uint32_t width,
    uint32_t height,
    VkFormat colorFormat
)
    : m_device(parentDevice), m_width(width), m_height(height), m_colorFormat(colorFormat) {
	m_depthFormat = m_device.FindDepthFormat();
	if (m_depthFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("No suitable depth format found!");
	}

	VkDevice device = m_device.GetDevice();

	CreateImages();

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

	if (vkCreateRenderPass(device, &rpInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass for framebuffer");
	}

	std::array<VkImageView, 2> fbAttachments = { m_colorImageView, m_depthImageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = m_renderPass;
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

	if (m_renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, m_renderPass, nullptr);
	}

	if (m_framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, m_framebuffer, nullptr);
	}

	FreeImages();

	m_sampler.reset();
}

VkFramebuffer VulkanFrameBuffer::GetFrameBuffer() const {
	return m_framebuffer;
}

VkRenderPass VulkanFrameBuffer::GetRenderPass() const {
	return m_renderPass;
}

VkSampler VulkanFrameBuffer::GetSampler() const {
	if (m_sampler == nullptr) {
		return VK_NULL_HANDLE;
	}
	return m_sampler->GetSampler();
}

VkImageView VulkanFrameBuffer::GetColorImageView() const {
	return m_colorImageView;
}

uint32_t VulkanFrameBuffer::GetWidth() const {
	return m_width;
}

uint32_t VulkanFrameBuffer::GetHeight() const {
	return m_height;
}

void VulkanFrameBuffer::Resize(uint32_t width, uint32_t height) {
	if (m_width == width && m_height == height) {
		return;
	}

	VkDevice device = m_device.GetDevice();

	if (m_framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, m_framebuffer, nullptr);
		m_framebuffer = VK_NULL_HANDLE;
	}

	FreeImages();

	m_width = width;
	m_height = height;

	CreateImages();

	std::array<VkImageView, 2> attachments = { m_colorImageView, m_depthImageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = m_renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbInfo.pAttachments = attachments.data();
	fbInfo.width = width;
	fbInfo.height = height;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(device, &fbInfo, nullptr, &m_framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to recreate framebuffer during resize!");
	}
}

void VulkanFrameBuffer::CreateImages() {
	m_device.CreateImage(
	    m_width,
	    m_height,
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
	    m_width,
	    m_height,
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
}

void VulkanFrameBuffer::FreeImages() {
	VkDevice device = m_device.GetDevice();
	if (m_colorImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_colorImageView, nullptr);
		m_colorImageView = VK_NULL_HANDLE;
	}
	if (m_colorImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_colorImage, nullptr);
		m_colorImage = VK_NULL_HANDLE;
	}
	if (m_colorImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_colorImageMemory, nullptr);
		m_colorImageMemory = VK_NULL_HANDLE;
	}

	if (m_depthImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_depthImageView, nullptr);
		m_depthImageView = VK_NULL_HANDLE;
	}
	if (m_depthImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_depthImage, nullptr);
		m_depthImage = VK_NULL_HANDLE;
	}
	if (m_depthImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_depthImageMemory, nullptr);
		m_depthImageMemory = VK_NULL_HANDLE;
	}
}

} // namespace PixieRenderer
