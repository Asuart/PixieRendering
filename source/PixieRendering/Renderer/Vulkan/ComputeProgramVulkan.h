#pragma once
#include <vulkan/vulkan.h>

namespace PixieRenderer {

struct ComputeProgramVulkan {
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
};

} // namespace PixieRenderer
