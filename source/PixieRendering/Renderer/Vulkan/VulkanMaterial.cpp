#include "VulkanMaterial.h"

#include "VulkanDevice.h"

namespace PixieRenderer {

VulkanMaterial::VulkanMaterial(VulkanDevice& parentDevice, const Material* materialInfo)
    : m_device(parentDevice) {
	VkDevice device = m_device.GetDevice();

	CompiledShader shader = ShaderCompilerVulkan::CompileShader(
	    device,
	    materialInfo->vertexShaderSource,
	    materialInfo->fragmentShaderSource
	);

	m_shaderStages = shader.stages;
	m_shaderStagesCreateInfo = shader.stagesCreateInfo;
	m_bindingsInfo = shader.bindingsInfo;
	for (const auto& binding : shader.bindingsInfo.bindings) {
		material.nameToBinding[binding.name] = binding.binding;
	}

	CreateMaterialDescriptorSetLayout(material.bindingsInfo.bindings, material.descriptorSetLayout);
	CreateMaterialPipelineLayout(material.descriptorSetLayout, material.pipelineLayout);

	VkPipeline pipeline;
	CreateMaterialPipeline(
	    material.pipelineLayout,
	    m_renderPass,
	    shader.stagesCreateInfo.data(),
	    static_cast<uint32_t>(shader.stagesCreateInfo.size()),
	    pipeline
	);
	material.pipelines[m_renderPass] = pipeline;

	// for (size_t i = 0; i < shader.stages.size(); i++) {
	//	vkDestroyShaderModule(m_device, shader.stages[i], nullptr);
	// }

	CreateMaterialDescriptorPool(material.bindingsInfo.bindings, material.descriptorPool);

	material.descriptorSets.resize(cMaxFramesInFlight);
	for (uint32_t i = 0; i < cMaxFramesInFlight; i++) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = material.descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &material.descriptorSetLayout;
		if (vkAllocateDescriptorSets(device, &allocInfo, &material.descriptorSets[i]) !=
		    VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	for (const ShaderBinding& b : shader.bindingsInfo.bindings) {
		if (b.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
		    b.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
			uint32_t binding = b.binding;
			uint32_t blockSize = b.size;

			std::vector<BufferResourceVulkan> buffers(cMaxFramesInFlight);
			for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
				BufferResourceVulkan& res = buffers[frame];
				CreateBuffer(
				    blockSize,
				    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				    res.buffer,
				    res.bufferMemory
				);
				vkMapMemory(device, res.bufferMemory, 0, blockSize, 0, &res.bufferMapped);
				res.size = blockSize;
			}
			material.uniformBuffers[binding] = buffers;
		}
	}

	for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
		std::vector<VkWriteDescriptorSet> writes;

		for (const auto& [binding, buffers] : material.uniformBuffers) {
			const BufferResourceVulkan& res = buffers[frame];
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = res.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = res.size;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = material.descriptorSets[frame];
			write.dstBinding = binding;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.pBufferInfo = &bufferInfo;
			writes.push_back(write);
		}

		if (!writes.empty()) {
			vkUpdateDescriptorSets(
			    device,
			    static_cast<uint32_t>(writes.size()),
			    writes.data(),
			    0,
			    nullptr
			);
		}
	}
}

VulkanMaterial::~VulkanMaterial() {
	VkDevice device = m_device.GetDevice();

	for (auto& [binding, buffers] : m_uniformBuffers) {
		for (uint32_t frame = 0; frame < cMaxFramesInFlight; ++frame) {
			const auto& res = buffers[frame];
			if (res.buffer != VK_NULL_HANDLE) {
				vkUnmapMemory(device, res.bufferMemory);
				vkDestroyBuffer(device, res.buffer, nullptr);
				vkFreeMemory(device, res.bufferMemory, nullptr);
			}
		}
	}
	m_uniformBuffers.clear();

	if (m_descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
		m_descriptorPool = VK_NULL_HANDLE;
	}

	for (auto& entry : m_pipelines) {
		vkDestroyPipeline(device, entry.second, nullptr);
	}
	m_pipelines.clear();

	for (VkShaderModule module : m_shaderStages) {
		vkDestroyShaderModule(device, module, nullptr);
	}
	m_shaderStages.clear();

	vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
}

} // namespace PixieRenderer
