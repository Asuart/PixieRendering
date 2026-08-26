#include "VulkanMesh.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanMesh::VulkanMesh(VulkanDevice& parentDevice) : m_device(parentDevice) {

}

VulkanMesh::~VulkanMesh() {
	VkDevice device = m_device.GetDevice();
	
	if (meshEntry.indexBuffer != VK_NULL_HANDLE) {
		m_device.FreeBuffer(meshEntry.indexBuffer, meshEntry.indexBufferMemory);
		meshEntry.indexBuffer = VK_NULL_HANDLE;
		meshEntry.indexBufferMemory = VK_NULL_HANDLE;
	}

	if (meshEntry.vertexBuffer != VK_NULL_HANDLE) {
		m_device.FreeBuffer(meshEntry.vertexBuffer, meshEntry.vertexBufferMemory);
		meshEntry.vertexBuffer = VK_NULL_HANDLE;
		meshEntry.vertexBufferMemory = VK_NULL_HANDLE;
	}
}

void VulkanMesh::Load(const Mesh* mesh) {
	m_device.CreateBuffer(
	    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
	    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    meshEntry.indexBuffer,
	    meshEntry.indexBufferMemory
	);
	m_device.LoadBuffer(
	    meshEntry.indexBuffer,
	    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
	    reinterpret_cast<const void*>(mesh->indexes.data())
	);

	m_device.CreateBuffer(
	    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
	    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    meshEntry.vertexBuffer,
	    meshEntry.vertexBufferMemory
	);
	m_device.LoadBuffer(
	    meshEntry.vertexBuffer,
	    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
	    reinterpret_cast<const void*>(mesh->vertexes.data())
	);
}


} // namespace PixieRenderer
