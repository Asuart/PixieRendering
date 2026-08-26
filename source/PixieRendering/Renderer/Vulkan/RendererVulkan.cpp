#include "RendererVulkan.h"

#include <algorithm>
#include <set>

#include "../../Window/WindowVulkan.h"
#include "DebugVulkan.h"

namespace PixieRenderer {

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

RendererVulkan::RendererVulkan(Window* window) : IRenderer(window, RenderAPI::Vulkan) {
	InitVulkan();
	m_surfaceWidth = window->GetResolution().x;
	m_surfaceHeight = window->GetResolution().y;
	m_viewportStart = { 0, 0 };

	m_viewportResolution = { static_cast<int>(m_surfaceWidth), static_cast<int>(m_surfaceHeight) };
}

void RendererVulkan::InitVulkan() {
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	InitializeDevice();

	SwapChainSupportDetails swapChainSupport = m_device.QuerySwapChainSupport();
	m_device.CreateRenderPass(VulkanSwapchain::ChooseSurfaceFormat(swapChainSupport.formats).format, m_renderPass);

	m_swapchain = m_device.CreateSwapchain({ m_surfaceWidth, m_surfaceHeight }, m_renderPass);

	m_device.CreateCommandPool(m_commandPool);

	m_commandBuffers.resize(cMaxFramesInFlight);
	for (int32_t i = 0; i < m_commandBuffers.size(); i++) {
		m_device.CreateCommandBuffer(m_commandPool, m_commandBuffers[i]);
	}

	CreateSyncObjects();
}

void RendererVulkan::Cleanup() {
	m_device.DestroySwapchain(std::move(m_swapchain));

	m_device.DestroyRenderPass(m_renderPass);

	for (size_t i = 0; i < cMaxFramesInFlight; i++) {
		m_device.DestroyFence(m_inFlightFences[i]);
		m_device.DestroySemaphore(m_imageAvailableSemaphores[i]);
		m_device.DestroySemaphore(m_renderFinishedSemaphores[i]);
	}

	m_device.DestroyCommandPool(m_commandPool);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void RendererVulkan::SetRenderResolution(uint32_t width, uint32_t height) {
	if (m_surfaceWidth == width && m_surfaceHeight == height) {
		return;
	}
	m_surfaceWidth = width;
	m_surfaceHeight = height;

	SetViewport({ 0, 0 }, { width, height });

	RecreateSwapChain();
}

void RendererVulkan::StartFrame() {
	m_device.WaitFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
	    m_device,
	    m_swapchain->m_swapchain,
	    UINT64_MAX,
	    m_imageAvailableSemaphores[m_currentFrame],
	    VK_NULL_HANDLE,
	    &m_nextImageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	m_device.ResetFences(1, &m_inFlightFences[m_currentFrame]);
	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	m_renderRequests.clear();
}

void RendererVulkan::EndFrame() {
	if (vkEndCommandBuffer(m_commandBuffers[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_device.m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSwapchainKHR swapChains[] = { m_swapchain->m_swapchain };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_nextImageIndex;

	VkResult result = vkQueuePresentKHR(m_device.m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		RecreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % cMaxFramesInFlight;

	WaitIdle();
}

void RendererVulkan::BeginRenderPass() {
	VkRenderPass renderPass = m_renderPass;
	VkFramebuffer framebuffer = m_swapchain->m_framebuffers[m_nextImageIndex];
	VkExtent2D extent = m_swapchain->m_extent;

	if (m_activeFrameBuffer.id != -1) {
		auto& fb = GetFrameBufferEntry(m_activeFrameBuffer);
		renderPass = fb.renderPass;
		framebuffer = fb.framebuffer;
		extent = { fb.width, fb.height };
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.extent = extent;
	renderPassInfo.renderArea.offset = { 0, 0 };

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(
	    m_commandBuffers[m_currentFrame],
	    &renderPassInfo,
	    VK_SUBPASS_CONTENTS_INLINE
	);

	VkViewport viewport{};
	viewport.x = static_cast<float>(m_viewportStart.x);
	viewport.y = static_cast<float>(m_viewportStart.y);
	viewport.width = (m_viewportResolution.x > 0) ? static_cast<float>(m_viewportResolution.x)
	                                              : static_cast<float>(extent.width);
	viewport.height = (m_viewportResolution.y > 0) ? static_cast<float>(m_viewportResolution.y)
	                                               : static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { m_viewportStart.x, m_viewportStart.y };
	scissor.extent = { (m_viewportResolution.x > 0) ? m_viewportResolution.x : extent.width,
		               (m_viewportResolution.y > 0) ? m_viewportResolution.y : extent.height };
	vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);

	m_renderRequests.clear();
}

void RendererVulkan::EndRenderPass() {
	for (const RenderRequest& request : m_renderRequests) {
		MeshVulkan& mesh = GetMeshEntry(request.meshHandle);
		MaterialVulkan& material = GetMaterialEntry(request.materialHandle);

		VkRenderPass currentRenderPass = (m_activeFrameBuffer.id != -1)
		                                     ? GetFrameBufferEntry(m_activeFrameBuffer).renderPass
		                                     : m_renderPass;

		auto it = material.pipelines.find(currentRenderPass);
		if (it == material.pipelines.end()) {
			VkPipeline pipeline;
			CreateMaterialPipeline(
			    material.pipelineLayout,
			    currentRenderPass,
			    material.shaderStagesCreateInfo.data(),
			    static_cast<uint32_t>(material.shaderStagesCreateInfo.size()),
			    pipeline
			);
			it = material.pipelines.emplace(currentRenderPass, pipeline).first;
		}
		vkCmdBindPipeline(
		    m_commandBuffers[m_currentFrame],
		    VK_PIPELINE_BIND_POINT_GRAPHICS,
		    it->second
		);

		std::vector<VkBuffer> vertexBuffers = { mesh.vertexBuffer };
		std::vector<VkDeviceSize> offsets = { 0 };

		vkCmdBindVertexBuffers(
		    m_commandBuffers[m_currentFrame],
		    0,
		    static_cast<uint32_t>(offsets.size()),
		    vertexBuffers.data(),
		    offsets.data()
		);

		vkCmdBindIndexBuffer(
		    m_commandBuffers[m_currentFrame],
		    mesh.indexBuffer,
		    0,
		    VK_INDEX_TYPE_UINT32
		);

		vkCmdBindDescriptorSets(
		    m_commandBuffers[m_currentFrame],
		    VK_PIPELINE_BIND_POINT_GRAPHICS,
		    material.pipelineLayout,
		    0,
		    1,
		    &material.descriptorSets[m_currentFrame],
		    0,
		    nullptr
		);

		vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], mesh.indicesCount, 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);

	MemoryBarriersAll();
}

MeshHandle RendererVulkan::CreateMesh(const Mesh* mesh) {
	MeshVulkan meshEntry;
	m_meshes.push_back(meshEntry);

	MeshHandle handle = MeshHandle(static_cast<uint32_t>(m_meshes.size() - 1));
	if (mesh != nullptr) {
		LoadMesh(handle, mesh);
	}

	return handle;
}

void RendererVulkan::DestroyMesh(MeshHandle handle) {
	MeshVulkan& meshEntry = GetMeshEntry(handle);

	meshEntry.indicesCount = 0;

	if (meshEntry.indexBuffer != VK_NULL_HANDLE) {
		m_device.FreeBuffer(meshEntry.indexBuffer, meshEntry.indexBufferMemory);
		meshEntry.indexBuffer = VK_NULL_HANDLE;
		meshEntry.indexBufferMemory = VK_NULL_HANDLE;
	}

	if (meshEntry.vertexBuffer != VK_NULL_HANDLE) {
		m_device.FreeBuffer(meshEntry.vertexBuffer, meshEntry.vertexBufferMemory);
		meshEntry.vertexBuffer = VK_NULL_HANDLE;
		meshEntry.vertexBufferMemory = VK_NULL_HANDLE;
	}
}

void RendererVulkan::LoadMesh(MeshHandle handle, const Mesh* mesh) {
	MeshVulkan& meshEntry = GetMeshEntry(handle);

	DestroyMesh(handle);

	meshEntry.indicesCount = static_cast<uint32_t>(mesh->indexes.size());

	if (mesh->indexes.size() > 0) {
		m_device.CreateBuffer(
		    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    meshEntry.indexBuffer,
		    meshEntry.indexBufferMemory
		);
		m_device.LoadBuffer(
		    meshEntry.indexBuffer,
		    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
		    reinterpret_cast<const void*>(mesh->indexes.data())
		);
	}

	if (mesh->vertexes.size() > 0) {
		m_device.CreateBuffer(
		    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    meshEntry.vertexBuffer,
		    meshEntry.vertexBufferMemory
		);
		m_device.LoadBuffer(
		    meshEntry.vertexBuffer,
		    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
		    reinterpret_cast<const void*>(mesh->vertexes.data())
		);
	}
}

void RendererVulkan::DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) {
	m_renderRequests.push_back({ meshHandle, materialHandle });
}

FrameBufferHandle RendererVulkan::CreateFrameBuffer(glm::uvec2 resolution) {
	resolution = glm::clamp(resolution, { 1, 1 }, resolution);

	FrameBufferVulkan fb;


	m_frameBuffers.push_back(fb);
	return FrameBufferHandle(m_frameBuffers.size() - 1);
}

void RendererVulkan::DestroyFrameBuffer(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	if (fb.framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(m_device, fb.framebuffer, nullptr);
		vkDestroyRenderPass(m_device, fb.renderPass, nullptr);
		DestroyTexture(fb.colorTexture);
		DestroyTexture(fb.depthTexture);
	}
}

void RendererVulkan::ResizeFrameBuffer(FrameBufferHandle handle, glm::uvec2 resolution) {
	resolution = glm::clamp(resolution, { 1, 1 }, resolution);

	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	if (fb.width == resolution.x && fb.height == resolution.y) {
		return;
	}

	WaitIdle();

	VulkanTexture& colorTexture = GetTextureEntry(fb.colorTexture);
	VulkanTexture& depthTexture = GetTextureEntry(fb.depthTexture);

	if (fb.framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(m_device, fb.framebuffer, nullptr);
		fb.framebuffer = VK_NULL_HANDLE;
	}

	m_device.DestroyTexture(colorTexture);
	m_device.DestroyTexture(depthTexture);

	fb.width = resolution.x;
	fb.height = resolution.y;

	colorTexture.width = resolution.x;
	colorTexture.height = resolution.y;

	depthTexture.width = resolution.x;
	depthTexture.height = resolution.y;

	m_device.CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    colorTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    colorTexture.image,
	    colorTexture.memory
	);
	m_device.CreateImageView(
	    colorTexture.image,
	    colorTexture.format,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    1,
	    colorTexture.imageView
	);

	m_device.CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthTexture.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    depthTexture.image,
	    depthTexture.memory
	);
	m_device.CreateImageView(
	    depthTexture.image,
	    depthTexture.format,
	    VK_IMAGE_ASPECT_DEPTH_BIT,
	    1,
	    depthTexture.imageView
	);

	std::array<VkImageView, 2> attachments = { colorTexture.imageView, depthTexture.imageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = fb.renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbInfo.pAttachments = attachments.data();
	fbInfo.width = resolution.x;
	fbInfo.height = resolution.y;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &fb.framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to recreate framebuffer during resize!");
	}
}

void RendererVulkan::BindFrameBuffer(FrameBufferHandle handle) {
	m_activeFrameBuffer = handle;
}

void RendererVulkan::UnbindFrameBuffer() {
	m_activeFrameBuffer = FrameBufferHandle();
}

TextureHandle RendererVulkan::CreateTexture(const Image2D* image) {
	VulkanTexture textureEntry;

	m_textures.push_back(textureEntry);

	TextureHandle handle = TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
	LoadTexture(handle, image);

	return handle;
}

void RendererVulkan::DestroyTexture(TextureHandle handle) {
	VulkanTexture& texture = GetTextureEntry(handle);
	m_device.DestroyTexture(texture);
	texture = VulkanTexture();
}

void RendererVulkan::LoadTexture(TextureHandle handle, const Image2D* image) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);

	textureEntry.format = ToVkFormat(image->format);

	DestroyTexture(handle);

	VkDeviceSize imageSize = image->resolution.x * image->resolution.y * 4;
	textureEntry.mipLevels =
	    static_cast<uint32_t>(
	        std::floor(std::log2(std::max(image->resolution.x, image->resolution.y)))
	    ) +
	    1;
	textureEntry.width = image->resolution.x;
	textureEntry.height = image->resolution.y;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_device.CreateBuffer(
	    imageSize,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	    stagingBuffer,
	    stagingBufferMemory
	);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, image->pixels.data(), static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	m_device.CreateImage(
	    image->resolution.x,
	    image->resolution.y,
	    textureEntry.mipLevels,
	    VK_SAMPLE_COUNT_1_BIT,
	    textureEntry.format,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	        VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    textureEntry.image,
	    textureEntry.memory
	);

	m_device.TransitionImageLayout(
	    textureEntry.image,
	    textureEntry.format,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    textureEntry.mipLevels
	);

	m_device.CopyBufferToImage(
	    stagingBuffer,
	    textureEntry.image,
	    image->resolution.x,
	    image->resolution.y
	);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	m_device.GenerateMipmaps(
	    textureEntry.image,
	    textureEntry.format,
	    image->resolution.x,
	    image->resolution.y,
	    textureEntry.mipLevels
	);

	// Image view
	m_device.CreateImageView(
	    textureEntry.image,
	    textureEntry.format,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    textureEntry.mipLevels,
	    textureEntry.imageView
	);

	// Image sampler
	m_device.CreateSampler(textureEntry, textureEntry.sampler);
}

void RendererVulkan::SetTextureFiltering(
    TextureHandle handle,
    TextureFiltering minFilter,
    TextureFiltering magFilter
) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);

	textureEntry.minFilter = ToVkFilter(minFilter);
	textureEntry.magFilter = ToVkFilter(magFilter);

	m_device.DestroySampler(textureEntry.sampler);
	m_device.CreateSampler(textureEntry, textureEntry.sampler);
}

void RendererVulkan::SetTextureWrap(
    TextureHandle handle,
    TextureWrap wrapU,
    TextureWrap wrapV,
    TextureWrap wrapW
) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);

	textureEntry.addressModeU = ToVkSamplerAddressMode(wrapU);
	textureEntry.addressModeV = ToVkSamplerAddressMode(wrapV);
	textureEntry.addressModeW = ToVkSamplerAddressMode(wrapW);

	m_device.DestroySampler(textureEntry.sampler);
	m_device.CreateSampler(textureEntry, textureEntry.sampler);
}

void RendererVulkan::GenerateTextureMipmaps(TextureHandle handle) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	GenerateMipmaps(
	    textureEntry.image,
	    textureEntry.format,
	    textureEntry.width,
	    textureEntry.height,
	    textureEntry.mipLevels
	);
}

glm::ivec2 RendererVulkan::GetTextureResolution(TextureHandle handle) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	return glm::ivec2(textureEntry.width, textureEntry.height);
}

void RendererVulkan::BindTexture(
    MaterialHandle materialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	MaterialVulkan& material = GetMaterialEntry(materialHandle);

	auto it = material.nameToBinding.find(name);
	if (it == material.nameToBinding.end()) {
		throw std::runtime_error("Texture binding not found: " + name);
	}

	uint32_t binding = it->second;
	VulkanTexture& tex = GetTextureEntry(textureHandle);

	for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = tex.imageView;
		imageInfo.sampler = tex.sampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = material.descriptorSets[frame];
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
	}

	material.textureBindings[binding] = textureHandle;
}

void RendererVulkan::BindTexture(
    ComputeProgramHandle computeMaterialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	ComputeProgramVulkan& prog = GetComputeProgramEntry(computeMaterialHandle);

	auto it = prog.nameToBinding.find(name);
	if (it == prog.nameToBinding.end()) {
		throw std::runtime_error("Texture binding not found in compute program: " + name);
	}
	uint32_t binding = it->second;

	VulkanTexture& tex = GetTextureEntry(textureHandle);

	for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = tex.imageView;
		imageInfo.sampler = tex.sampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = prog.descriptorSets[frame];
		write.dstBinding = binding;
		write.dstArrayElement = index;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
	}
}

ShaderStorageBufferHandle
RendererVulkan::CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) {
	ShaderStorageBufferVulkan buf;
	buf.size = size;

	m_device.CreateBuffer(
	    size,
	    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    buf.buffer,
	    buf.bufferMemory
	);

	m_shaderStorageBuffers.push_back(buf);

	ShaderStorageBufferHandle handle(static_cast<uint32_t>(m_shaderStorageBuffers.size() - 1));

	if (data != nullptr) {
		LoadShaderStorageBuffer(handle, data, size);
	}

	return handle;
}

void RendererVulkan::DestroyShaderStorageBuffer(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferVulkan& buf = GetShaderStorageBufferEntry(handle);
	if (buf.buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(m_device, buf.buffer, nullptr);
		vkFreeMemory(m_device, buf.bufferMemory, nullptr);
		buf.buffer = VK_NULL_HANDLE;
		buf.bufferMemory = VK_NULL_HANDLE;
		buf.size = 0;
	}
}

void RendererVulkan::LoadShaderStorageBuffer(
    ShaderStorageBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	ShaderStorageBufferVulkan& buf = GetShaderStorageBufferEntry(handle);
	m_device.LoadBuffer(buf.buffer, size, data);
}

uint32_t RendererVulkan::GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferVulkan& buf = GetShaderStorageBufferEntry(handle);
	return static_cast<uint32_t>(buf.size);
}

std::vector<uint8_t> RendererVulkan::GetShaderStorageBufferData(
    ShaderStorageBufferHandle handle,
    uint32_t offset,
    uint32_t size
) {
	ShaderStorageBufferVulkan& buf = GetShaderStorageBufferEntry(handle);
	if (offset + size > buf.size) {
		throw std::runtime_error("Requested data range out of bounds");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	m_device.CreateBuffer(
	    size,
	    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	    stagingBuffer,
	    stagingMemory
	);

	VkCommandBuffer cmd = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = offset;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, buf.buffer, stagingBuffer, 1, &copyRegion);

	EndSingleTimeCommands(cmd);

	void* mapped;
	vkMapMemory(m_device, stagingMemory, 0, size, 0, &mapped);
	std::vector<uint8_t> result(size);
	memcpy(result.data(), mapped, size);
	vkUnmapMemory(m_device, stagingMemory);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingMemory, nullptr);

	return result;
}

UniformBufferHandle RendererVulkan::CreateUniformBuffer(const uint8_t* data, uint32_t size) {
	UniformBufferVulkan buf;
	buf.size = size;

	m_device.CreateBuffer(
	    size,
	    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	    buf.buffer,
	    buf.bufferMemory
	);

	vkMapMemory(m_device, buf.bufferMemory, 0, size, 0, &buf.bufferMapped);

	if (data != nullptr) {
		memcpy(buf.bufferMapped, data, size);
	}

	m_uniformBuffers.push_back(buf);

	return UniformBufferHandle(static_cast<uint32_t>(m_uniformBuffers.size() - 1));
}

void RendererVulkan::DestroyUniformBuffer(UniformBufferHandle handle) {
	UniformBufferVulkan& buf = GetUniformBufferEntry(handle);
	if (buf.buffer != VK_NULL_HANDLE) {
		vkUnmapMemory(m_device, buf.bufferMemory);
		vkDestroyBuffer(m_device, buf.buffer, nullptr);
		vkFreeMemory(m_device, buf.bufferMemory, nullptr);

		buf.size = 0;
		buf.buffer = VK_NULL_HANDLE;
		buf.bufferMemory = VK_NULL_HANDLE;
		buf.bufferMapped = nullptr;
	}
}

void RendererVulkan::LoadUniformBuffer(
    UniformBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	UniformBufferVulkan bufferEntry = GetUniformBufferEntry(handle);
	memcpy(bufferEntry.bufferMapped, data, size);
}

void RendererVulkan::LoadUniformBuffer(
    MaterialHandle handle,
    const std::string& name,
    const void* data,
    size_t size
) {
	MaterialVulkan& material = GetMaterialEntry(handle);

	auto it = material.nameToBinding.find(name);
	if (it == material.nameToBinding.end()) {
		throw std::runtime_error("Uniform binding not found: " + name);
	}

	uint32_t binding = it->second;
	auto& buffers = material.uniformBuffers[binding];
	auto& res = buffers[m_currentFrame];
	if (size > res.size) {
		throw std::runtime_error("Data size exceeds uniform buffer size");
	}

	memcpy(res.bufferMapped, data, size);
}

void RendererVulkan::CreateMaterialDescriptorSetLayout(
    const std::vector<ShaderBinding>& bindings,
    VkDescriptorSetLayout& outDescriptorSetLayout
) {
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (const ShaderBinding& b : bindings) {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = b.binding;
		binding.descriptorType = static_cast<VkDescriptorType>(b.type);
		binding.descriptorCount = b.count;
		binding.stageFlags = b.stageFlags;
		layoutBindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &outDescriptorSetLayout) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void RendererVulkan::CreateMaterialDescriptorPool(
    const std::vector<ShaderBinding>& bindings,
    VkDescriptorPool& outDescriptorPool
) {
	std::unordered_map<VkDescriptorType, uint32_t> poolSizeCounts;
	for (const ShaderBinding& b : bindings) {
		poolSizeCounts[static_cast<VkDescriptorType>(b.type)] += b.count * cMaxFramesInFlight;
	}

	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(poolSizeCounts.size());
	for (const auto& [type, count] : poolSizeCounts) {
		poolSizes.push_back({ type, count });
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = cMaxFramesInFlight;

	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &outDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool for material!");
	}
}

void RendererVulkan::CreateMaterialPipelineLayout(
    VkDescriptorSetLayout descriptorSetLayout,
    VkPipelineLayout& outPipelineLayout
) {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &outPipelineLayout) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void RendererVulkan::CreateMaterialPipeline(
    VkPipelineLayout pipelineLayout,
    VkRenderPass renderPass,
    const VkPipelineShaderStageCreateInfo* shaderStages,
    uint32_t shaderStagesCount,
    VkPipeline& outPipeline
) {
	auto bindingDescriptions = GetMeshBindingDescriptions();
	auto attributeDescriptions = GetMeshAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount =
	    static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount =
	    static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
		                                          VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStagesCount;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(
	        m_device,
	        VK_NULL_HANDLE,
	        1,
	        &pipelineInfo,
	        nullptr,
	        &outPipeline
	    ) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

MaterialHandle RendererVulkan::CreateMaterial(const Material* materialInfo) {
	MaterialVulkan material;

	CompiledShader shader = ShaderCompilerVulkan::CompileShader(
	    m_device,
	    materialInfo->vertexShaderSource,
	    materialInfo->fragmentShaderSource
	);

	material.shaderStages = shader.stages;
	material.shaderStagesCreateInfo = shader.stagesCreateInfo;
	material.bindingsInfo = shader.bindingsInfo;
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
		if (vkAllocateDescriptorSets(m_device, &allocInfo, &material.descriptorSets[i]) !=
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
				vkMapMemory(m_device, res.bufferMemory, 0, blockSize, 0, &res.bufferMapped);
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
			    m_device,
			    static_cast<uint32_t>(writes.size()),
			    writes.data(),
			    0,
			    nullptr
			);
		}
	}

	m_materials.push_back(std::move(material));

	return MaterialHandle(static_cast<uint32_t>(m_materials.size() - 1));
}

void RendererVulkan::DestroyMaterial(MaterialHandle handle) {
	MaterialVulkan& material = GetMaterialEntry(handle);

	for (auto& [binding, buffers] : material.uniformBuffers) {
		for (uint32_t frame = 0; frame < cMaxFramesInFlight; ++frame) {
			const auto& res = buffers[frame];
			if (res.buffer != VK_NULL_HANDLE) {
				vkUnmapMemory(m_device, res.bufferMemory);
				vkDestroyBuffer(m_device, res.buffer, nullptr);
				vkFreeMemory(m_device, res.bufferMemory, nullptr);
			}
		}
	}
	material.uniformBuffers.clear();

	if (material.descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, material.descriptorPool, nullptr);
		material.descriptorPool = VK_NULL_HANDLE;
	}

	for (auto& entry : material.pipelines) {
		vkDestroyPipeline(m_device, entry.second, nullptr);
	}
	material.pipelines.clear();

	for (VkShaderModule module : material.shaderStages) {
		vkDestroyShaderModule(m_device, module, nullptr);
	}
	material.shaderStages.clear();

	vkDestroyPipelineLayout(m_device, material.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, material.descriptorSetLayout, nullptr);
}

ComputeProgramHandle RendererVulkan::CreateComputeProgram(const char* source) {
	ComputeProgramVulkan prog;

	CompiledComputeShader compiled = ShaderCompilerVulkan::CompileComputeShader(m_device, source);
	prog.bindingsInfo = compiled.bindingsInfo;

	CreateMaterialDescriptorSetLayout(compiled.bindingsInfo.bindings, prog.descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &prog.descriptorSetLayout;
	vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &prog.pipelineLayout);

	CreateMaterialDescriptorPool(compiled.bindingsInfo.bindings, prog.descriptorPool);

	prog.descriptorSets.resize(cMaxFramesInFlight);
	for (uint32_t i = 0; i < cMaxFramesInFlight; i++) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = prog.descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &prog.descriptorSetLayout;
		if (vkAllocateDescriptorSets(m_device, &allocInfo, &prog.descriptorSets[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets for compute program!");
		}
	}

	for (const ShaderBinding& b : compiled.bindingsInfo.bindings) {
		if (b.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
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
				vkMapMemory(m_device, res.bufferMemory, 0, blockSize, 0, &res.bufferMapped);
				res.size = blockSize;
			}
			prog.uniformBuffers[binding] = buffers;
		}
	}

	for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
		std::vector<VkWriteDescriptorSet> writes;
		for (const auto& [binding, buffers] : prog.uniformBuffers) {
			const BufferResourceVulkan& res = buffers[frame];
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = res.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = res.size;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = prog.descriptorSets[frame];
			write.dstBinding = binding;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.pBufferInfo = &bufferInfo;
			writes.push_back(write);
		}
		if (!writes.empty()) {
			vkUpdateDescriptorSets(
			    m_device,
			    static_cast<uint32_t>(writes.size()),
			    writes.data(),
			    0,
			    nullptr
			);
		}
	}

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = prog.pipelineLayout;
	pipelineInfo.stage = compiled.stageCreateInfo;
	vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &prog.pipeline);

	m_computePrograms.push_back(std::move(prog));

	return ComputeProgramHandle(static_cast<uint32_t>(m_computePrograms.size() - 1));
}

void RendererVulkan::DestroyComputeProgram(ComputeProgramHandle handle) {
	ComputeProgramVulkan& prog = GetComputeProgramEntry(handle);

	if (prog.pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(m_device, prog.pipeline, nullptr);
		prog.pipeline = VK_NULL_HANDLE;
	}

	if (prog.pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_device, prog.pipelineLayout, nullptr);
		prog.pipelineLayout = VK_NULL_HANDLE;
	}

	if (prog.descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, prog.descriptorPool, nullptr);
		prog.descriptorPool = VK_NULL_HANDLE;
	}

	if (prog.descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(m_device, prog.descriptorSetLayout, nullptr);
		prog.descriptorSetLayout = VK_NULL_HANDLE;
	}

	prog.descriptorSets.clear();
	prog.nameToBinding.clear();
}

void RendererVulkan::DispatchComputeProgram(
    ComputeProgramHandle handle,
    int32_t x,
    int32_t y,
    int32_t z
) {
	ComputeProgramVulkan& prog = GetComputeProgramEntry(handle);
	vkCmdBindPipeline(
	    m_commandBuffers[m_currentFrame],
	    VK_PIPELINE_BIND_POINT_COMPUTE,
	    prog.pipeline
	);
	vkCmdBindDescriptorSets(
	    m_commandBuffers[m_currentFrame],
	    VK_PIPELINE_BIND_POINT_COMPUTE,
	    prog.pipelineLayout,
	    0,
	    1,
	    &prog.descriptorSets[m_currentFrame],
	    0,
	    nullptr
	);
	vkCmdDispatch(m_commandBuffers[m_currentFrame], x, y, z);
}

void RendererVulkan::SetViewport(glm::ivec2 start, glm::ivec2 resolution) {
	m_viewportStart = start;
	m_viewportResolution = resolution;
}

void RendererVulkan::WaitIdle() {
	m_device.WaitIdle();
}

void RendererVulkan::MemoryBarriersAll() {
	VkMemoryBarrier memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

	vkCmdPipelineBarrier(
	    m_commandBuffers[m_currentFrame],
	    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	    0,
	    1,
	    &memoryBarrier,
	    0,
	    nullptr,
	    0,
	    nullptr
	);
}

TextureHandle RendererVulkan::GetColorAttachmentHandle(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	return fb.colorTexture;
}

TextureHandle RendererVulkan::GetDepthAttachmentHandle(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	return fb.depthTexture;
}

uint64_t RendererVulkan::GetInternalID(TextureHandle handle) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	return reinterpret_cast<uint64_t>(textureEntry.image);
}

uint64_t RendererVulkan::GetInternalColorAttachmentID(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	VulkanTexture& texture = GetTextureEntry(fb.colorTexture);
	return reinterpret_cast<uint64_t>(texture.imageView);
}

uint64_t RendererVulkan::GetInternalDepthAttachmentID(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	VulkanTexture& texture = GetTextureEntry(fb.depthTexture);
	return reinterpret_cast<uint64_t>(texture.imageView);
}

VkInstance RendererVulkan::GetInstance() const {
	return m_instance;
}

VkPhysicalDevice RendererVulkan::GetPhysicalDevice() const {
	return m_device.m_physicalDevice;
}

VkDevice RendererVulkan::GetDevice() const {
	return m_device.m_device;
}

VkQueue RendererVulkan::GetGraphicsQueue() const {
	return m_device.m_graphicsQueue;
}

VkQueue RendererVulkan::GetPresentQueue() const {
	return m_device.m_graphicsQueue;
}

VkCommandBuffer RendererVulkan::GetCommandBuffer() const {
	return m_commandBuffers[m_currentFrame];
}

VkRenderPass RendererVulkan::GetRenderPass() const {
	return m_renderPass;
}

VulkanTexture& RendererVulkan::GetTextureEntry(TextureHandle handle) {
	return m_textures[handle.id];
}

MeshVulkan& RendererVulkan::GetMeshEntry(MeshHandle handle) {
	return m_meshes[handle.id];
}

MaterialVulkan& RendererVulkan::GetMaterialEntry(MaterialHandle handle) {
	return m_materials[handle.id];
}

ComputeProgramVulkan& RendererVulkan::GetComputeProgramEntry(ComputeProgramHandle handle) {
	return m_computePrograms[handle.id];
}

UniformBufferVulkan& RendererVulkan::GetUniformBufferEntry(UniformBufferHandle handle) {
	return m_uniformBuffers[handle.id];
}

ShaderStorageBufferVulkan&
RendererVulkan::GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle) {
	return m_shaderStorageBuffers[handle.id];
}

FrameBufferVulkan& RendererVulkan::GetFrameBufferEntry(FrameBufferHandle handle) {
	return m_frameBuffers[handle.id];
}

VkImageView RendererVulkan::GetTextureImageView(TextureHandle handle) {
	VulkanTexture& texture = GetTextureEntry(handle);
	return texture.imageView;
}

VkSampler RendererVulkan::GetTextureSmapler(TextureHandle handle) {
	VulkanTexture& texture = GetTextureEntry(handle);
	return texture.sampler;
}

void RendererVulkan::CreateInstance() {
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "PixieEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	WindowVulkan* windowVulkan = reinterpret_cast<WindowVulkan*>(m_window);
	std::vector<const char*> requiredExtensions = windowVulkan->GetRequiredExtensions();
	if (enableValidationLayers) {
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void RendererVulkan::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo
) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

void RendererVulkan::CreateSurface() {
	WindowVulkan* windowVulkan = reinterpret_cast<WindowVulkan*>(m_window);
	windowVulkan->CreateSurface(m_instance, m_surface);
}

void RendererVulkan::SetupDebugMessenger() {
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void RendererVulkan::InitializeDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	m_device.Initialize(physicalDevice, m_surface);
}

bool RendererVulkan::IsDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = VulkanDevice::FindQueueFamilies(device, m_surface);

	bool extensionsSupported = VulkanDevice::CheckExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
		    VulkanDevice::QuerySwapChainSupport(device, m_surface);
		swapChainAdequate =
		    !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate &&
	       supportedFeatures.samplerAnisotropy;
}

std::vector<VkVertexInputBindingDescription> RendererVulkan::GetMeshBindingDescriptions() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return { bindingDescription };
}

std::vector<VkVertexInputAttributeDescription> RendererVulkan::GetMeshAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, uv);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SINT;
	attributeDescriptions[3].offset = offsetof(Vertex, boneIDs);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, boneWeights);

	return attributeDescriptions;
}

void RendererVulkan::CreateSyncObjects() {
	m_imageAvailableSemaphores.resize(cMaxFramesInFlight);
	m_renderFinishedSemaphores.resize(cMaxFramesInFlight);
	m_inFlightFences.resize(cMaxFramesInFlight);
	for (size_t i = 0; i < cMaxFramesInFlight; i++) {
		m_device.CreateFence(m_inFlightFences[i]);
		m_device.CreateSemaphore(m_imageAvailableSemaphores[i]);
		m_device.CreateSemaphore(m_renderFinishedSemaphores[i]);
	}
}

void RendererVulkan::RecreateSwapChain() {
	m_device.WaitIdle();
	m_device.DestroySwapchain(std::move(m_swapchain));
	m_swapchain = m_device.CreateSwapchain({ m_surfaceWidth, m_surfaceHeight }, m_renderPass);
}

bool RendererVulkan::CheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}

	return true;
}

} // namespace PixieRenderer
