#pragma once
#include <vector>

#include <vulkan/vulkan.h>

#include "UniformBufferVulkan.h"
#include "ShaderCompilationVulkan.h"
#include "RendererVulkan.h"

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
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::unordered_map<uint32_t, std::array<BufferResource, RendererVulkan::cMaxFramesInFlight>>
	    uniformBuffers;
	BindingsInfo bindingsInfo;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::unordered_map<std::string, uint32_t> nameToBinding;
	std::unordered_map<uint32_t, TextureHandle> textureBindings;
};

} // namespace PixieRenderer
