#pragma once
#include <vector>

#include "VulkanBuffer.h"
#include "VulkanComputeProgram.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGraphicsProgram.h"
#include "VulkanMesh.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"

namespace PixieRenderer {

class VulkanResourceManager {
  public:
	explicit VulkanResourceManager(VulkanDevice& device);

	template <typename... Args> MeshHandle CreateMesh(Args&&... args) {
		m_meshes.push_back(std::make_unique<VulkanMesh>(m_device, std::forward<Args>(args)...));
		return MeshHandle(static_cast<uint32_t>(m_meshes.size() - 1));
	}

	template <typename... Args> TextureHandle CreateTexture(Args&&... args) {
		m_textures.push_back(std::make_unique<VulkanTexture>(m_device, std::forward<Args>(args)...)
		);
		return TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
	}

	template <typename... Args> MaterialHandle CreateGraphicsProgram(Args&&... args) {
		m_graphicsPrograms.push_back(
		    std::make_unique<VulkanGraphicsProgram>(m_device, std::forward<Args>(args)...)
		);
		return MaterialHandle(static_cast<uint32_t>(m_graphicsPrograms.size() - 1));
	}

	template <typename... Args> ComputeProgramHandle CreateComputeProgram(Args&&... args) {
		m_computePrograms.push_back(
		    std::make_unique<VulkanComputeProgram>(m_device, std::forward<Args>(args)...)
		);
		return ComputeProgramHandle(static_cast<uint32_t>(m_computePrograms.size() - 1));
	}

	template <typename... Args> UniformBufferHandle CreateUniformBuffer(Args&&... args) {
		m_uniformBuffers.push_back(
		    std::make_unique<VulkanBuffer>(m_device, std::forward<Args>(args)...)
		);
		return UniformBufferHandle(static_cast<uint32_t>(m_uniformBuffers.size() - 1));
	}

	template <typename... Args>
	ShaderStorageBufferHandle CreateShaderStorageBuffer(Args&&... args) {
		m_shaderStorageBuffers.push_back(
		    std::make_unique<VulkanBuffer>(m_device, std::forward<Args>(args)...)
		);
		return ShaderStorageBufferHandle(static_cast<uint32_t>(m_shaderStorageBuffers.size() - 1));
	}

	template <typename... Args> FrameBufferHandle CreateFrameBuffer(Args&&... args) {
		m_frameBuffers.push_back(
		    std::make_unique<VulkanFrameBuffer>(m_device, std::forward<Args>(args)...)
		);
		return FrameBufferHandle(static_cast<uint32_t>(m_frameBuffers.size() - 1));
	}

	VulkanTexture& GetTextureEntry(TextureHandle handle);
	VulkanMesh& GetMeshEntry(MeshHandle handle);
	VulkanGraphicsProgram& GetGraphicsProgramEntry(MaterialHandle handle);
	VulkanComputeProgram& GetComputeProgramEntry(ComputeProgramHandle handle);
	VulkanBuffer& GetUniformBufferEntry(UniformBufferHandle handle);
	VulkanBuffer& GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle);
	VulkanFrameBuffer& GetFrameBufferEntry(FrameBufferHandle handle);

	std::vector<std::unique_ptr<VulkanMesh>>& GetMeshes();
	std::vector<std::unique_ptr<VulkanGraphicsProgram>>& GetGraphicsPrograms();

  private:
	VulkanDevice& m_device;

	std::vector<std::unique_ptr<VulkanMesh>> m_meshes = {};
	std::vector<std::unique_ptr<VulkanTexture>> m_textures = {};
	std::vector<std::unique_ptr<VulkanGraphicsProgram>> m_graphicsPrograms = {};
	std::vector<std::unique_ptr<VulkanComputeProgram>> m_computePrograms = {};
	std::vector<std::unique_ptr<VulkanBuffer>> m_uniformBuffers = {};
	std::vector<std::unique_ptr<VulkanBuffer>> m_shaderStorageBuffers = {};
	std::vector<std::unique_ptr<VulkanFrameBuffer>> m_frameBuffers = {};
};

} // namespace PixieRenderer
