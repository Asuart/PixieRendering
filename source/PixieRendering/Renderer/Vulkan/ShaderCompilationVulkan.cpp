#include "ShaderCompilationVulkan.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace PixieRenderer {

ShaderBinding::ShaderBinding(
    uint32_t _binding,
    uint32_t _set,
    VkDescriptorType _type,
    uint32_t _count,
    uint32_t _stageFlags,
    uint32_t _blockSize,
    const std::string& _name
)
    : binding(_binding),
      set(_set),
      type(_type),
      count(_count),
      stageFlags(_stageFlags),
      blockSize(_blockSize),
      name(_name) {
}

void ShaderCompilerVulkan::Initialize() {
	if (!isInitialized) {
		glslang_initialize_process();
		isInitialized = true;
	}
}

void ShaderCompilerVulkan::Free() {
	if (isInitialized) {
		glslang_finalize_process();
		isInitialized = false;
	}
}

SpirVBinary CompileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char* shaderSource) {
	const glslang_input_t input = {
		.language = GLSLANG_SOURCE_GLSL,
		.stage = stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_0,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_0,
		.code = shaderSource,
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = glslang_default_resource(),
	};

	glslang_shader_t* shader = glslang_shader_create(&input);

	SpirVBinary bin = {
		.words = NULL,
		.size = 0,
	};

	if (!glslang_shader_preprocess(shader, &input)) {
		printf("GLSL preprocessing failed.\n");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", input.code);
		glslang_shader_delete(shader);
		return bin;
	}

	if (!glslang_shader_parse(shader, &input)) {
		printf("GLSL parsing failed\n");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", glslang_shader_get_preprocessed_code(shader));
		glslang_shader_delete(shader);
		return bin;
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		printf("GLSL linking failed\n");
		printf("%s\n", glslang_program_get_info_log(program));
		printf("%s\n", glslang_program_get_info_debug_log(program));
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return bin;
	}

	glslang_program_SPIRV_generate(program, stage);

	bin.size = static_cast<int32_t>(glslang_program_SPIRV_get_size(program));
	bin.words = new uint32_t(bin.size);
	glslang_program_SPIRV_get(program, bin.words);

	const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
	if (spirv_messages) {
		printf("%s\b", spirv_messages);
	}

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	return bin;
}

VkShaderModule ShaderCompilerVulkan::CreateShaderModule(VkDevice device, SpirVBinary binary) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = binary.size * sizeof(binary.words[0]);
	createInfo.pCode = reinterpret_cast<const uint32_t*>(binary.words);

	VkShaderModule shaderModule = nullptr;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw "Failed to create vulkan shader module";
	}

	return shaderModule;
}

CompiledShader ShaderCompilerVulkan::CompileShader(
    VkDevice device,
    const char* vertexShaderSource,
    const char* fragmentShaderSource
) {

	SpirVBinary vertexBinary =
	    CompileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, vertexShaderSource);
	SpirVBinary fragmentBinary =
	    CompileShaderToSPIRV_Vulkan(GLSLANG_STAGE_FRAGMENT, fragmentShaderSource);

	BindingsInfo vertexInfo = ReflectSPIRV(vertexBinary);
	BindingsInfo fragmentInfo = ReflectSPIRV(fragmentBinary);

	std::vector<ShaderBinding> mergedBindings =
	    MergeBindings(vertexInfo.bindings, fragmentInfo.bindings);

	BindingsInfo finalInfo;
	finalInfo.bindings = std::move(mergedBindings);
	for (const auto& binding : finalInfo.bindings) {
		if (binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
		    binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
			finalInfo.uniformBufferBindings.push_back(binding.binding);
		}
	}

	VkShaderModule vertShaderModule = CreateShaderModule(device, vertexBinary);
	VkShaderModule fragShaderModule = CreateShaderModule(device, fragmentBinary);

	delete[] vertexBinary.words;
	delete[] fragmentBinary.words;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	return { { vertShaderModule, fragShaderModule },
		     { vertShaderStageInfo, fragShaderStageInfo },
		     finalInfo };
}

CompiledComputeShader
ShaderCompilerVulkan::CompileComputeShader(VkDevice device, const char* source) {
	SpirVBinary computeBinary = CompileShaderToSPIRV_Vulkan(GLSLANG_STAGE_COMPUTE, source);
	VkShaderModule shaderModule = CreateShaderModule(device, computeBinary);
	delete[] computeBinary.words;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	vertShaderStageInfo.module = shaderModule;
	vertShaderStageInfo.pName = "main";

	return { shaderModule, vertShaderStageInfo };
}

BindingsInfo ShaderCompilerVulkan::ReflectSPIRV(const SpirVBinary& binary) {
	BindingsInfo result;

	spirv_cross::CompilerGLSL compiler(binary.words, binary.size);
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	auto add_bindings =
	    [&](const auto& resource_list, VkDescriptorType type, VkShaderStageFlagBits stage) {
		    std::vector<uint32_t> bindingIndexes{};

		    for (const auto& res : resource_list) {
			    const auto& binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			    bindingIndexes.push_back(binding);

			    const auto& set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
			    uint32_t setIndex = (set != 0) ? set : 0;

			    auto type_id = compiler.get_type(res.type_id);
			    size_t blockSize = compiler.get_declared_struct_size(type_id);

			    uint32_t count = 1;
			    if (type_id.array.size() > 0) {
				    count = type_id.array[0];
			    }

			    result.bindings.push_back(
			        ShaderBinding(binding, setIndex, type, count, stage, blockSize, res.name)
			    );
		    }

		    return bindingIndexes;
	    };

	result.uniformBufferBindings = add_bindings(
	    resources.uniform_buffers,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    VK_SHADER_STAGE_ALL
	);
	add_bindings(resources.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL);
	add_bindings(
	    resources.sampled_images,
	    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	    VK_SHADER_STAGE_ALL
	);
	add_bindings(resources.separate_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL);
	add_bindings(resources.separate_samplers, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL);

	return result;
}

std::vector<ShaderBinding> ShaderCompilerVulkan::MergeBindings(
    const std::vector<ShaderBinding>& a,
    const std::vector<ShaderBinding>& b
) {
	std::unordered_map<uint64_t, ShaderBinding> map;

	auto add = [&](const ShaderBinding& src) {
		uint64_t key = (static_cast<uint64_t>(src.set) << 32) | src.binding;
		auto it = map.find(key);
		if (it == map.end()) {
			map[key] = src;
		} else {
			if (it->second.type != src.type) {
				throw std::runtime_error(
				    "Binding type mismatch for set " + std::to_string(src.set) + " binding " +
				    std::to_string(src.binding)
				);
			}
			if (it->second.count != src.count) {
				throw std::runtime_error(
				    "Binding count mismatch for set " + std::to_string(src.set) + " binding " +
				    std::to_string(src.binding)
				);
			}
			if (it->second.blockSize != src.blockSize) {
				throw std::runtime_error(
				    "Block size mismatch for set " + std::to_string(src.set) + " binding " +
				    std::to_string(src.binding)
				);
			}
			it->second.stageFlags |= src.stageFlags;
		}
	};

	for (const auto& s : a)
		add(s);
	for (const auto& s : b)
		add(s);

	std::vector<ShaderBinding> result;
	result.reserve(map.size());
	for (auto& [key, binding] : map) {
		result.push_back(binding);
	}

	std::sort(result.begin(), result.end(), [](const ShaderBinding& lhs, const ShaderBinding& rhs) {
		return lhs.binding < rhs.binding;
	});

	return result;
}

} // namespace PixieRenderer
