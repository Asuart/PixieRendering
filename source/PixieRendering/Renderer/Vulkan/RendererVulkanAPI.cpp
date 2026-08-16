#include "RendererVulkan.h"

#include "ShaderCompilationVulkan.h"

namespace PixieRenderer {

void RendererVulkan::StartFrame() {
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
	    m_device,
	    m_swapChain,
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

	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPass renderPass = m_renderPass;
	VkFramebuffer framebuffer = m_swapChainFramebuffers[m_nextImageIndex];
	VkExtent2D extent = m_swapChainExtent;

	if (m_activeFrameBuffer.id != -1) {
		auto& fb = GetFrameBufferEntry(m_activeFrameBuffer);
		renderPass = fb.renderPass;
		framebuffer = fb.framebuffer;
		extent = { static_cast<uint32_t>(fb.resolution.x), static_cast<uint32_t>(fb.resolution.y) };
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

void RendererVulkan::EndFrame() {
	for (const RenderRequest& request : m_renderRequests) {
		const MeshVulkan& mesh = GetMeshEntry(request.meshHandle);
		const MaterialVulkan& material = GetMaterialEntry(request.materialHandle);

		VkRenderPass currentRenderPass = (m_activeFrameBuffer.id != -1)
		                                     ? GetFrameBufferEntry(m_activeFrameBuffer).renderPass
		                                     : m_renderPass;

		auto it = material.pipelines.find(currentRenderPass);
		if (it == material.pipelines.end()) {
			VkPipeline pipeline = CreatePipeline();
			// Создать новый pipeline для этого render pass
			// Можно повторно использовать pipelineInfo, заменив renderPass
			VkGraphicsPipelineCreateInfo newInfo = material.pipelineInfo;
			newInfo.renderPass = currentRenderPass;
			VkPipeline newPipeline;
			if (vkCreateGraphicsPipelines(
			        m_device,
			        VK_NULL_HANDLE,
			        1,
			        &newInfo,
			        nullptr,
			        &newPipeline
			    ) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline for custom render pass");
			}
			it = material.pipelines.emplace(currentRenderPass, newPipeline).first;
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
		    offsets.size(),
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

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSwapchainKHR swapChains[] = { m_swapChain };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_nextImageIndex;

	VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		RecreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % cMaxFramesInFlight;

	WaitIdle();
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
		FreeBuffer(meshEntry.indexBuffer, meshEntry.indexBufferMemory);
		meshEntry.indexBuffer = VK_NULL_HANDLE;
		meshEntry.indexBufferMemory = VK_NULL_HANDLE;
	}

	if (meshEntry.vertexBuffer != VK_NULL_HANDLE) {
		FreeBuffer(meshEntry.vertexBuffer, meshEntry.vertexBufferMemory);
		meshEntry.vertexBuffer = VK_NULL_HANDLE;
		meshEntry.vertexBufferMemory = VK_NULL_HANDLE;
	}
}

void RendererVulkan::LoadMesh(MeshHandle handle, const Mesh* mesh) {
	MeshVulkan& meshEntry = GetMeshEntry(handle);

	DestroyMesh(handle);

	meshEntry.indicesCount = static_cast<uint32_t>(mesh->indexes.size());

	if (mesh->indexes.size() > 0) {
		CreateBuffer(
		    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    meshEntry.indexBuffer,
		    meshEntry.indexBufferMemory
		);
		LoadBuffer(
		    meshEntry.indexBuffer,
		    sizeof(mesh->indexes[0]) * mesh->indexes.size(),
		    reinterpret_cast<const void*>(mesh->indexes.data())
		);
	}

	if (mesh->vertexes.size() > 0) {
		CreateBuffer(
		    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    meshEntry.vertexBuffer,
		    meshEntry.vertexBufferMemory
		);
		LoadBuffer(
		    meshEntry.vertexBuffer,
		    sizeof(mesh->vertexes[0]) * mesh->vertexes.size(),
		    reinterpret_cast<const void*>(mesh->vertexes.data())
		);
	}
}

void RendererVulkan::DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) {
	m_renderRequests.push_back({ meshHandle, materialHandle });
}

FrameBufferHandle RendererVulkan::CreateFrameBuffer(glm::ivec2 resolution) {
	FrameBufferVulkan fb;
	fb.resolution = resolution;

	VkFormat colorFormat = m_swapChainImageFormat;
	VkFormat depthFormat = FindDepthFormat();

	CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    colorFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    fb.colorImage,
	    fb.colorImageMemory
	);
	fb.colorImageView = CreateImageView(fb.colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	CreateImage(
	    resolution.x,
	    resolution.y,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    depthFormat,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    fb.depthImage,
	    fb.depthImageMemory
	);
	fb.depthImageView = CreateImageView(fb.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthRef{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.attachmentCount = attachments.size();
	rpInfo.pAttachments = attachments.data();
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;

	VkRenderPass renderPass;
	if (vkCreateRenderPass(m_device, &rpInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass for framebuffer");
	}

	std::array<VkImageView, 2> fbAttachments = { fb.colorImageView, fb.depthImageView };
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = fbAttachments.size();
	fbInfo.pAttachments = fbAttachments.data();
	fbInfo.width = resolution.x;
	fbInfo.height = resolution.y;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &fb.framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer");
	}

	fb.renderPass = renderPass;

	m_frameBuffers.push_back(fb);
	return FrameBufferHandle(static_cast<uint32_t>(m_frameBuffers.size() - 1));
}

void RendererVulkan::DestroyFrameBuffer(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	if (fb.framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(m_device, fb.framebuffer, nullptr);
		vkDestroyRenderPass(m_device, fb.renderPass, nullptr);
		vkDestroyImageView(m_device, fb.colorImageView, nullptr);
		vkDestroyImage(m_device, fb.colorImage, nullptr);
		vkFreeMemory(m_device, fb.colorImageMemory, nullptr);
		vkDestroyImageView(m_device, fb.depthImageView, nullptr);
		vkDestroyImage(m_device, fb.depthImage, nullptr);
		vkFreeMemory(m_device, fb.depthImageMemory, nullptr);
	}
}

void RendererVulkan::ResizeFrameBuffer(FrameBufferHandle handle, glm::ivec2 resolution) {
}

void RendererVulkan::BindFrameBuffer(FrameBufferHandle handle) {
	m_activeFrameBuffer = handle;
}

void RendererVulkan::UnbindFrameBuffer() {
	m_activeFrameBuffer = FrameBufferHandle(-1);
}

TextureHandle
RendererVulkan::CreateTexture(const uint8_t* data, glm::ivec2 resolution, TextureFormat format) {
	TextureVulkan textureEntry;

	m_textures.push_back(textureEntry);

	TextureHandle handle = TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
	LoadTexture(handle, data, resolution, format);

	return handle;
}

void RendererVulkan::DestroyTexture(TextureHandle handle) {
	TextureVulkan& texture = GetTextureEntry(handle);
	if (texture.textureImage != VK_NULL_HANDLE) {
		vkDestroySampler(m_device, texture.textureSampler, nullptr);
		texture.textureSampler = VK_NULL_HANDLE;

		vkDestroyImageView(m_device, texture.textureImageView, nullptr);
		texture.textureImageView = VK_NULL_HANDLE;

		vkDestroyImage(m_device, texture.textureImage, nullptr);
		texture.textureImage = VK_NULL_HANDLE;

		vkFreeMemory(m_device, texture.textureImageMemory, nullptr);
		texture.textureImageMemory = VK_NULL_HANDLE;
	}
}

void RendererVulkan::LoadTexture(
    TextureHandle handle,
    const uint8_t* pixels,
    glm::ivec2 resolution,
    TextureFormat format
) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);

	DestroyTexture(handle);

	VkDeviceSize imageSize = resolution.x * resolution.y * 4;
	textureEntry.mipLevels =
	    static_cast<uint32_t>(std::floor(std::log2(std::max(resolution.x, resolution.y)))) + 1;
	textureEntry.width = resolution.x;
	textureEntry.height = resolution.y;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
	    imageSize,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	    stagingBuffer,
	    stagingBufferMemory
	);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	CreateImage(
	    resolution.x,
	    resolution.y,
	    textureEntry.mipLevels,
	    VK_SAMPLE_COUNT_1_BIT,
	    ToVkFormat(format),
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	        VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	    textureEntry.textureImage,
	    textureEntry.textureImageMemory
	);

	TransitionImageLayout(
	    textureEntry.textureImage,
	    VK_FORMAT_R8G8B8A8_SRGB,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    textureEntry.mipLevels
	);

	CopyBufferToImage(stagingBuffer, textureEntry.textureImage, resolution.x, resolution.y);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	GenerateMipmaps(
	    textureEntry.textureImage,
	    VK_FORMAT_R8G8B8A8_SRGB,
	    resolution.x,
	    resolution.y,
	    textureEntry.mipLevels
	);

	// Image view
	textureEntry.textureImageView = CreateImageView(
	    textureEntry.textureImage,
	    VK_FORMAT_R8G8B8A8_SRGB,
	    VK_IMAGE_ASPECT_COLOR_BIT,
	    textureEntry.mipLevels
	);

	// Image sampler
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = textureEntry.magFilter;
	samplerInfo.minFilter = textureEntry.minFilter;
	samplerInfo.addressModeU = textureEntry.addressModeU;
	samplerInfo.addressModeV = textureEntry.addressModeV;
	samplerInfo.addressModeW = textureEntry.addressModeW;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &textureEntry.textureSampler) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void RendererVulkan::SetTextureFiltering(
    TextureHandle handle,
    TextureFiltering minFilter,
    TextureFiltering magFilter
) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);

	textureEntry.minFilter = ToVkFilter(minFilter);
	textureEntry.magFilter = ToVkFilter(magFilter);

	vkDestroySampler(m_device, textureEntry.textureSampler, nullptr);

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = textureEntry.magFilter;
	samplerInfo.minFilter = textureEntry.minFilter;
	samplerInfo.addressModeU = textureEntry.addressModeU;
	samplerInfo.addressModeV = textureEntry.addressModeV;
	samplerInfo.addressModeW = textureEntry.addressModeW;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &textureEntry.textureSampler) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to recreate texture sampler!");
	}
}

void RendererVulkan::SetTextureWrap(TextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);

	textureEntry.addressModeU = ToVkSamplerAddressMode(wrapS);
	textureEntry.addressModeV = ToVkSamplerAddressMode(wrapT);

	vkDestroySampler(m_device, textureEntry.textureSampler, nullptr);

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = textureEntry.magFilter;
	samplerInfo.minFilter = textureEntry.minFilter;
	samplerInfo.addressModeU = textureEntry.addressModeU;
	samplerInfo.addressModeV = textureEntry.addressModeV;
	samplerInfo.addressModeW = textureEntry.addressModeW;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &textureEntry.textureSampler) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to recreate texture sampler!");
	}
}

void RendererVulkan::GenerateTextureMipmaps(TextureHandle handle) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);
	GenerateMipmaps(
	    textureEntry.textureImage,
	    VK_FORMAT_R8G8B8A8_SRGB,
	    textureEntry.width,
	    textureEntry.height,
	    textureEntry.mipLevels
	);
}

glm::ivec2 RendererVulkan::GetTextureResolution(TextureHandle handle) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);
	return glm::ivec2(textureEntry.width, textureEntry.height);
}

void RendererVulkan::BindTexture(
    MaterialHandle materialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint64_t index
) {
	MaterialVulkan& material = GetMaterialEntry(materialHandle);

	auto it = material.nameToBinding.find(name);
	if (it == material.nameToBinding.end()) {
		throw std::runtime_error("Texture binding not found: " + name);
	}

	uint32_t binding = it->second;
	TextureVulkan& tex = GetTextureEntry(textureHandle);

	for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = tex.textureImageView;
		imageInfo.sampler = tex.textureSampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = material.descriptorSets[frame];
		write.dstBinding = binding;
		write.dstArrayElement = 0;
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
    uint64_t index
) {
	throw "Nout implemented";
}

ShaderStorageBufferHandle
RendererVulkan::CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) {
	ShaderStorageBufferVulkan buf;
	buf.size = size;

	CreateBuffer(
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

	LoadBuffer(buf.buffer, size, data);
}

uint32_t RendererVulkan::GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) {
	ShaderStorageBufferVulkan& buf = GetShaderStorageBufferEntry(handle);
	return buf.size;
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
	CreateBuffer(
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

	CreateBuffer(
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

VkPipeline RendererVulkan::CreatePipeline(const Material* materialInfo) {
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
	multisampling.rasterizationSamples = m_msaaSamples;

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
	pipelineInfo.stageCount = static_cast<uint32_t>(shader.stagesCreateInfo.size());
	pipelineInfo.pStages = shader.stagesCreateInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = material.pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	return pipeline;
}

MaterialHandle
RendererVulkan::CreateMaterial(const Material* materialInfo) {
	MaterialVulkan material;

	CompiledShader shader = ShaderCompilerVulkan::CompileShader(
	    m_device,
	    materialInfo->vertexShaderSource,
	    materialInfo->fragmentShaderSource
	);
	material.bindingsInfo = shader.bindingsInfo;

	for (const auto& binding : shader.bindingsInfo.bindings) {
		material.nameToBinding[binding.name] = binding.binding;
	}

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (const ShaderBinding& b : shader.bindingsInfo.bindings) {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = b.binding;
		binding.descriptorType = b.type;
		binding.descriptorCount = b.count;
		binding.stageFlags = b.stageFlags;
		layoutBindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(
	        m_device,
	        &layoutInfo,
	        nullptr,
	        &material.descriptorSetLayout
	    ) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &material.descriptorSetLayout;

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &material.pipelineLayout) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}



	material.pipelines[m_renderPass] = pipeline;

	for (size_t i = 0; i < shader.stages.size(); i++) {
		vkDestroyShaderModule(m_device, shader.stages[i], nullptr);
	}

	std::unordered_map<VkDescriptorType, uint32_t> poolSizeCounts;
	for (const ShaderBinding& b : shader.bindingsInfo.bindings) {
		poolSizeCounts[b.type] += b.count * cMaxFramesInFlight;
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

	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &material.descriptorPool) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool for material!");
	}

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
			uint32_t blockSize = b.blockSize;

			std::vector<BufferResource> buffers(cMaxFramesInFlight);
			for (uint32_t frame = 0; frame < cMaxFramesInFlight; frame++) {
				BufferResource& res = buffers[frame];
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
			const BufferResource& res = buffers[frame];
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

	vkDestroyPipeline(m_device, material.pipeline, nullptr);
	vkDestroyPipelineLayout(m_device, material.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, material.descriptorSetLayout, nullptr);
}

ComputeProgramHandle RendererVulkan::CreateComputeProgram(const char* source) {
	// ComputeProgramVulkan program;

	// VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	// pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// pipelineLayoutInfo.setLayoutCount = 1;
	// pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

	// if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &program.pipelineLayout)
	// !=
	//     VK_SUCCESS) {
	//	throw std::runtime_error("failed to create compute pipeline layout!");
	// }

	// CompiledComputeShader computeShader =
	//     ShaderCompilerVulkan::CompileComputeShader(m_device, source);

	// VkComputePipelineCreateInfo pipelineInfo{};
	// pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	// pipelineInfo.layout = program.pipelineLayout;
	// pipelineInfo.stage = computeShader.stageCreateInfo;

	// if (vkCreateComputePipelines(
	//         m_device,
	//         VK_NULL_HANDLE,
	//         1,
	//         &pipelineInfo,
	//         nullptr,
	//         &program.pipeline
	//     ) != VK_SUCCESS) {
	//	throw std::runtime_error("failed to create compute pipeline!");
	// }

	return ComputeProgramHandle();
}

void RendererVulkan::DestroyComputeProgram(ComputeProgramHandle handle) {
	throw "Nout implemented";
}

void RendererVulkan::DispatchComputeProgram(
    ComputeProgramHandle handle,
    int32_t x,
    int32_t y,
    int32_t z
) {
	// ComputeProgramVulkan& prog = GetComputeProgramEntry(handle);
	// vkCmdBindPipeline(
	//     m_commandBuffers[m_currentFrame],
	//     VK_PIPELINE_BIND_POINT_COMPUTE,
	//     prog.pipeline
	//);
	// vkCmdBindDescriptorSets(
	//     m_commandBuffers[m_currentFrame],
	//     VK_PIPELINE_BIND_POINT_COMPUTE,
	//     prog.pipelineLayout,
	//     0,
	//     1,
	//     &prog.descriptorSets[m_currentFrame],
	//     0,
	//     nullptr
	//);
	// vkCmdDispatch(m_commandBuffers[m_currentFrame], x, y, z);
}

void RendererVulkan::SetViewport(glm::ivec2 start, glm::ivec2 resolution) {
	m_viewportStart = start;
	m_viewportResolution = resolution;
}

void RendererVulkan::WaitIdle() {
	vkDeviceWaitIdle(m_device);
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

uint64_t RendererVulkan::GetInternalID(TextureHandle handle) {
	TextureVulkan& textureEntry = GetTextureEntry(handle);
	return reinterpret_cast<uint64_t>(textureEntry.textureImage);
}

uint64_t RendererVulkan::GetInternalColorAttachmentID(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	return reinterpret_cast<uint64_t>(fb.colorImage);
}

uint64_t RendererVulkan::GetInternalDepthAttachmentID(FrameBufferHandle handle) {
	FrameBufferVulkan& fb = GetFrameBufferEntry(handle);
	return reinterpret_cast<uint64_t>(fb.depthImage);
}

TextureVulkan& RendererVulkan::GetTextureEntry(TextureHandle handle) {
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

} // namespace PixieRenderer
