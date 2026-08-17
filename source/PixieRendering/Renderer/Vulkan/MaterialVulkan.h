#pragma once
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "ShaderCompilationVulkan.h"
#include "../../ResourceHandles.h"

namespace PixieRenderer {

struct BufferResource {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	void* bufferMapped = nullptr;
	uint32_t size = 0;
};

struct MaterialVulkan {
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets = {};
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	std::unordered_map<VkRenderPass, VkPipeline> pipelines;
	std::unordered_map<uint32_t, std::vector<BufferResource>> uniformBuffers;
	BindingsInfo bindingsInfo;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::unordered_map<std::string, uint32_t> nameToBinding;
	std::unordered_map<uint32_t, TextureHandle> textureBindings;
	std::vector<VkShaderModule> shaderStages;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo;
};

} // namespace PixieRenderer
