#pragma once

#include "../../Resources/Material.h"
#include "VulkanProgram.h"
#include <unordered_map>
#include <vector>

namespace PixieRenderer {

class VulkanDevice;

class VulkanGraphicsProgram : public VulkanProgram {
  public:
	VulkanGraphicsProgram(
	    VulkanDevice& device,
	    VkRenderPass renderPass,
	    const Material* materialInfo
	);
	~VulkanGraphicsProgram() override;

	VkPipeline GetOrCreatePipeline(VkRenderPass renderPass);

	const std::vector<VkShaderModule>& GetShaderModules() const {
		return m_shaderModules;
	}

  private:
	VkRenderPass m_defaultRenderPass = VK_NULL_HANDLE;
	std::unordered_map<VkRenderPass, VkPipeline> m_pipelines; 

	std::vector<VkShaderModule> m_shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageCreateInfos;

	void CreatePipeline(VkRenderPass renderPass);
};

} // namespace PixieRenderer
