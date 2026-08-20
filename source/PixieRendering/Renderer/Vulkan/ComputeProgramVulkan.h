#pragma once
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "ShaderCompilationVulkan.h"
#include "BufferResourceVulkan.h"

namespace PixieRenderer {

struct ComputeProgramVulkan {
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets = {};
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::unordered_map<uint32_t, std::vector<BufferResourceVulkan>> uniformBuffers;
	BindingsInfo bindingsInfo;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::unordered_map<std::string, uint32_t> nameToBinding = {};
	std::unordered_map<uint32_t, TextureHandle> textureBindings;
};

} // namespace PixieRenderer
