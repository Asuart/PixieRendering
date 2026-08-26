#include "VulkanBuffer.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanBuffer::VulkanBuffer(
    VulkanDevice& parentDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
)
    : m_device(parentDevice), m_size(size), m_usage(usage), m_properties(properties) {
	VkDevice device = m_device.GetDevice();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_device.FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &m_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	if (vkBindBufferMemory(device, m_buffer, m_memory, 0) != VK_SUCCESS) {
		throw std::runtime_error("failed to bind buffer memory!");
	}

	if (m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		if (vkMapMemory(device, m_memory, 0, size, 0, &m_mappedMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to map buffer memory!");
		}
	}
}

VulkanBuffer::~VulkanBuffer() {
	VkDevice device = m_device.GetDevice();
	if (m_mappedMemory != nullptr) {
		vkUnmapMemory(device, m_memory);
	}
	if (m_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, m_buffer, nullptr);
	}
	if (m_memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_memory, nullptr);
	}
}

VkBuffer VulkanBuffer::GetBuffer() const {
	return m_buffer;
}

VkDeviceSize VulkanBuffer::GetSize() const {
	return m_size;
}

void VulkanBuffer::Load(VkDeviceSize size, const void* bufferData) {
	if (m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		memcpy(m_mappedMemory, bufferData, (size_t)size);
		return;
	}

	VulkanBuffer stagingBuffer(
	    m_device,
	    size,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	memcpy(stagingBuffer.m_mappedMemory, bufferData, (size_t)size);

	m_device.CopyBuffer(stagingBuffer.m_buffer, m_buffer, size);
}

} // namespace PixieRenderer
