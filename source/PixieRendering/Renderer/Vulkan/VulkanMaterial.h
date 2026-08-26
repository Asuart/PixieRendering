#pragma once
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "../../ResourceHandles.h"
#include "../../Resources/Material.h"
#include "BufferResourceVulkan.h"
#include "ShaderCompilationVulkan.h"

namespace PixieRenderer {

class VulkanDevice;

struct VulkanMaterial {
  public:
	VulkanMaterial(VulkanDevice& parentDevice, const Material* materialInfo);
	~VulkanMaterial();

  private:
	VulkanDevice& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> m_descriptorSets = {};
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	std::unordered_map<VkRenderPass, VkPipeline> m_pipelines = {};
	std::unordered_map<uint32_t, std::vector<BufferResourceVulkan>> m_uniformBuffers = {};
	BindingsInfo m_bindingsInfo = {};
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	std::unordered_map<std::string, uint32_t> m_nameToBinding = {};
	std::unordered_map<uint32_t, TextureHandle> m_textureBindings = {};
	std::vector<VkShaderModule> m_shaderStages = {};
	std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo = {};
};

} // namespace PixieRenderer
