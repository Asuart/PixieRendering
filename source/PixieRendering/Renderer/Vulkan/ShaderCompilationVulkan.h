#pragma once
#include <array>
#include <cstdint>
#include <string>

#include <vulkan/vulkan.hpp>

namespace PixieRenderer {

struct ShaderBinding {
	uint32_t binding;
	uint32_t set;
	VkDescriptorType type;
	uint32_t count;
	uint32_t stageFlags;
	uint32_t blockSize;
	std::string name;

	ShaderBinding(
	    uint32_t _binding,
	    uint32_t _set,
	    VkDescriptorType _type,
	    uint32_t _count,
	    uint32_t _stageFlags,
	    uint32_t _blockSize,
	    const std::string& _name
	);
};

struct BindingsInfo {
	std::vector<ShaderBinding> bindings;
	std::vector<uint32_t> uniformBufferBindings;
};

struct CompiledShader {
	std::vector<VkShaderModule> stages;
	std::vector<VkPipelineShaderStageCreateInfo> stagesCreateInfo;
	BindingsInfo bindingsInfo;
};

struct CompiledComputeShader {
	VkShaderModule stage;
	VkPipelineShaderStageCreateInfo stageCreateInfo;
};

struct SpirVBinary {
	uint32_t* words;
	int32_t size;
};

class ShaderCompilerVulkan {
  public:
	static void Initialize();
	static void Free();

	static CompiledShader CompileShader(
	    VkDevice device,
	    const char* vertexShaderSource,
	    const char* framgentShaderSource
	);

	static CompiledComputeShader CompileComputeShader(VkDevice device, const char* source);

  private:
	static inline bool isInitialized = false;

	static VkShaderModule CreateShaderModule(VkDevice device, SpirVBinary binary);
	static BindingsInfo ReflectSPIRV(const SpirVBinary& binary);
	static std::vector<ShaderBinding>
	MergeBindings(const std::vector<ShaderBinding>& a, const std::vector<ShaderBinding>& b);
};

} // namespace PixieRenderer
