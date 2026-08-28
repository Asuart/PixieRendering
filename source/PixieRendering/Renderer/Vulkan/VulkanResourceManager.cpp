#include "VulkanResourceManager.h"

namespace PixieRenderer {

VulkanResourceManager::VulkanResourceManager(VulkanDevice& device) : m_device(device) {
}

VulkanTexture& VulkanResourceManager::GetTextureEntry(TextureHandle handle) {
	return *m_textures[handle.id];
}

VulkanMesh& VulkanResourceManager::GetMeshEntry(MeshHandle handle) {
	return *m_meshes[handle.id];
}

VulkanGraphicsProgram& VulkanResourceManager::GetGraphicsProgramEntry(MaterialHandle handle) {
	return *m_graphicsPrograms[handle.id];
}

VulkanComputeProgram& VulkanResourceManager::GetComputeProgramEntry(ComputeProgramHandle handle) {
	return *m_computePrograms[handle.id];
}

VulkanBuffer& VulkanResourceManager::GetUniformBufferEntry(UniformBufferHandle handle) {
	return *m_uniformBuffers[handle.id];
}

VulkanBuffer& VulkanResourceManager::GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle) {
	return *m_shaderStorageBuffers[handle.id];
}

VulkanFrameBuffer& VulkanResourceManager::GetFrameBufferEntry(FrameBufferHandle handle) {
	return *m_frameBuffers[handle.id];
}

std::vector<std::unique_ptr<VulkanMesh>>& VulkanResourceManager::GetMeshes() {
	return m_meshes;
}

std::vector<std::unique_ptr<VulkanGraphicsProgram>>& VulkanResourceManager::GetGraphicsPrograms() {
	return m_graphicsPrograms;
}

} // namespace PixieRenderer
