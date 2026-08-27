#include "RendererVulkan.h"

#include <algorithm>
#include <set>

#include "../../Window/WindowVulkan.h"
#include "DebugVulkan.h"
#include "VulkanConfig.h"

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
	m_device.CreateRenderPass(
	    VulkanSwapchain::ChooseSurfaceFormat(swapChainSupport.formats).format,
	    m_renderPass
	);

	m_swapchain = std::make_unique<VulkanSwapchain>(
	    m_device,
	    VkExtent2D{ m_surfaceWidth, m_surfaceHeight },
	    m_renderPass
	);

	m_device.CreateCommandPool(m_commandPool);

	m_commandBuffers.resize(cMaxFramesInFlight);
	for (int32_t i = 0; i < m_commandBuffers.size(); i++) {
		m_device.CreateCommandBuffer(m_commandPool, m_commandBuffers[i]);
	}

	CreateSyncObjects();
}

void RendererVulkan::Cleanup() {
	m_swapchain.reset();

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

bool RendererVulkan::BeginFrame() {
	if (m_swapchainNeedsRecreate) {
		RecreateSwapChain();
		m_swapchainNeedsRecreate = false;
	}

	m_device.WaitFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	m_device.ResetFences(1, &m_inFlightFences[m_currentFrame]);

	VkResult result = vkAcquireNextImageKHR(
	    m_device.GetDevice(),
	    m_swapchain->GetSwapChain(),
	    UINT64_MAX,
	    m_imageAvailableSemaphores[m_currentFrame],
	    VK_NULL_HANDLE,
	    &m_nextImageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return false;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin command buffer!");
	}

	return true;
}

void RendererVulkan::EndFrame() {
	if (vkEndCommandBuffer(m_commandBuffers[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_device.m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
	    VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_swapchain->GetSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_nextImageIndex;

	VkResult result = vkQueuePresentKHR(m_device.m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		m_swapchainNeedsRecreate = true;
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % cMaxFramesInFlight;
}

void RendererVulkan::BeginRenderPass() {
	VkRenderPass renderPass = m_renderPass;
	VkFramebuffer framebuffer = m_swapchain->GetFrameBuffer(m_nextImageIndex);
	VkExtent2D extent = m_swapchain->GetExtent();

	if (m_activeFrameBuffer.id != -1) {
		VulkanFrameBuffer& fb = GetFrameBufferEntry(m_activeFrameBuffer);
		renderPass = fb.GetRenderPass();
		framebuffer = fb.GetFrameBuffer();
		extent = { fb.GetWidth(), fb.GetHeight() };
	}

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.extent = extent;
	renderPassInfo.renderArea.offset = { 0, 0 };
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
		VulkanMesh& mesh = GetMeshEntry(request.meshHandle);
		VulkanGraphicsProgram& material = GetMaterialEntry(request.materialHandle);

		VkRenderPass currentRenderPass =
		    (m_activeFrameBuffer.id != -1)
		        ? GetFrameBufferEntry(m_activeFrameBuffer).GetRenderPass()
		        : m_renderPass;

		vkCmdBindPipeline(
		    m_commandBuffers[m_currentFrame],
		    VK_PIPELINE_BIND_POINT_GRAPHICS,
		    material.GetOrCreatePipeline(currentRenderPass)
		);

		VkBuffer vertexBuffer = mesh.GetVertexBuffer().GetBuffer();
		VkBuffer indexBuffer = mesh.GetIndexBuffer().GetBuffer();
		VkDeviceSize offset = 0;

		vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], 0, 1, &vertexBuffer, &offset);

		vkCmdBindIndexBuffer(
		    m_commandBuffers[m_currentFrame],
		    indexBuffer,
		    0,
		    VK_INDEX_TYPE_UINT32
		);

		vkCmdBindDescriptorSets(
		    m_commandBuffers[m_currentFrame],
		    VK_PIPELINE_BIND_POINT_GRAPHICS,
		    material.GetPipelineLayout(),
		    0,
		    1,
		    &material.GetDescriptorSets()[m_currentFrame],
		    0,
		    nullptr
		);

		vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], mesh.GetIndexCount(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);

	MemoryBarriersAll();
}

MeshHandle RendererVulkan::CreateMesh(const Mesh* mesh) {
	m_meshes.push_back(std::make_unique<VulkanMesh>(m_device, mesh));
	return MeshHandle(static_cast<uint32_t>(m_meshes.size() - 1));
}

void RendererVulkan::LoadMesh(MeshHandle handle, const Mesh* mesh) {
	VulkanMesh& meshEntry = GetMeshEntry(handle);
	meshEntry.Load(mesh);
}

void RendererVulkan::DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle) {
	m_renderRequests.push_back({ meshHandle, materialHandle });
}

FrameBufferHandle RendererVulkan::CreateFrameBuffer(glm::uvec2 resolution, TextureFormat format) {
	m_frameBuffers.push_back(std::make_unique<VulkanFrameBuffer>(
	    m_device,
	    resolution.x,
	    resolution.y,
	    ToVkFormat(format)
	));
	return FrameBufferHandle(m_frameBuffers.size() - 1);
}

void RendererVulkan::ResizeFrameBuffer(FrameBufferHandle handle, glm::uvec2 resolution) {
	VulkanFrameBuffer& fb = GetFrameBufferEntry(handle);
	fb.Resize(resolution.x, resolution.y);
}

void RendererVulkan::BindFrameBuffer(FrameBufferHandle handle) {
	m_activeFrameBuffer = handle;
}

void RendererVulkan::UnbindFrameBuffer() {
	m_activeFrameBuffer = FrameBufferHandle();
}

TextureHandle RendererVulkan::CreateTexture(const Image2D* image) {
	m_textures.push_back(std::make_unique<VulkanTexture>(m_device, image));
	return TextureHandle(static_cast<int32_t>(m_textures.size() - 1));
}

void RendererVulkan::LoadTexture(TextureHandle handle, const Image2D* image) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	textureEntry.Load(image);
}

void RendererVulkan::SetTextureFiltering(
    TextureHandle handle,
    TextureFiltering minFilter,
    TextureFiltering magFilter
) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	textureEntry
	    .SetFiltering(ToVkFilter(minFilter), ToVkFilter(magFilter), ToVkMipmapMode(minFilter));
}

void RendererVulkan::SetTextureWrap(
    TextureHandle handle,
    TextureWrap wrapU,
    TextureWrap wrapV,
    TextureWrap wrapW
) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	textureEntry.SetWrap(
	    ToVkSamplerAddressMode(wrapU),
	    ToVkSamplerAddressMode(wrapV),
	    ToVkSamplerAddressMode(wrapW)
	);
}

glm::ivec2 RendererVulkan::GetTextureResolution(TextureHandle handle) {
	VulkanTexture& textureEntry = GetTextureEntry(handle);
	return glm::ivec2(textureEntry.GetWidth(), textureEntry.GetHeight());
}

void RendererVulkan::BindTexture(
    MaterialHandle materialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	VulkanGraphicsProgram& material = GetMaterialEntry(materialHandle);
	VulkanTexture& texture = GetTextureEntry(textureHandle);
	material.BindTexture(name, textureHandle, texture, index);
}

void RendererVulkan::BindTexture(
    ComputeProgramHandle computeMaterialHandle,
    const std::string& name,
    TextureHandle textureHandle,
    uint32_t index
) {
	VulkanComputeProgram& prog = GetComputeProgramEntry(computeMaterialHandle);
	VulkanTexture& texture = GetTextureEntry(textureHandle);
	prog.BindTexture(name, textureHandle, texture, index);
}

ShaderStorageBufferHandle
RendererVulkan::CreateShaderStorageBuffer(const uint8_t* data, uint32_t size) {
	m_shaderStorageBuffers.push_back(std::make_unique<VulkanBuffer>(
	    m_device,
	    size,
	    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	));

	ShaderStorageBufferHandle handle(static_cast<uint32_t>(m_shaderStorageBuffers.size() - 1));

	if (data != nullptr) {
		LoadShaderStorageBuffer(handle, data, size);
	}

	return handle;
}

void RendererVulkan::LoadShaderStorageBuffer(
    ShaderStorageBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	VulkanBuffer& buf = GetShaderStorageBufferEntry(handle);
	buf.Load(data, size);
}

uint32_t RendererVulkan::GetShaderStorageBufferSize(ShaderStorageBufferHandle handle) {
	VulkanBuffer& buf = GetShaderStorageBufferEntry(handle);
	return static_cast<uint32_t>(buf.GetSize());
}

std::vector<uint8_t> RendererVulkan::GetShaderStorageBufferData(
    ShaderStorageBufferHandle handle,
    uint32_t offset,
    uint32_t size
) {
	VulkanBuffer& buf = GetShaderStorageBufferEntry(handle);
	if (offset + size > buf.GetSize()) {
		throw std::runtime_error("Requested data range out of bounds");
	}
	if (size == 0) {
		return {};
	}

	VulkanBuffer stagingBuffer(
	    m_device,
	    size,
	    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	VkCommandBuffer cmd = m_device.BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = offset;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, buf.GetBuffer(), stagingBuffer.GetBuffer(), 1, &copyRegion);

	m_device.EndSingleTimeCommands(cmd);

	std::vector<uint8_t> result(size);
	stagingBuffer.ReadData(result.data(), size);

	return result;
}

UniformBufferHandle RendererVulkan::CreateUniformBuffer(const uint8_t* data, uint32_t size) {
	m_uniformBuffers.push_back(std::make_unique<VulkanBuffer>(
	    m_device,
	    size,
	    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	));

	UniformBufferHandle handle =
	    UniformBufferHandle(static_cast<uint32_t>(m_uniformBuffers.size() - 1));

	if (data != nullptr) {
		LoadUniformBuffer(handle, data, size);
	}

	return handle;
}

void RendererVulkan::LoadUniformBuffer(
    UniformBufferHandle handle,
    const uint8_t* data,
    uint32_t size
) {
	VulkanBuffer bufferEntry = GetUniformBufferEntry(handle);
	bufferEntry.Load(data, size);
}

void RendererVulkan::LoadUniformBuffer(
    MaterialHandle handle,
    const std::string& name,
    const void* data,
    size_t size
) {
	VulkanGraphicsProgram& material = GetMaterialEntry(handle);
	VulkanBuffer* buffer = material.GetUniformBuffer(name, m_currentFrame);
	if (buffer) {
		buffer->Load(data, size);
	}
}

MaterialHandle RendererVulkan::CreateMaterial(const Material* materialInfo) {
	m_materials.push_back(
	    std::make_unique<VulkanGraphicsProgram>(m_device, m_renderPass, materialInfo)
	);
	return MaterialHandle(static_cast<uint32_t>(m_materials.size() - 1));
}

ComputeProgramHandle RendererVulkan::CreateComputeProgram(const char* source) {
	m_computePrograms.push_back(
	    std::make_unique<VulkanComputeProgram>(m_device, std::string(source))
	);
	return ComputeProgramHandle(static_cast<uint32_t>(m_computePrograms.size() - 1));
}

void RendererVulkan::DispatchComputeProgram(
    ComputeProgramHandle handle,
    int32_t x,
    int32_t y,
    int32_t z
) {
	VulkanComputeProgram& prog = GetComputeProgramEntry(handle);
	VkCommandBuffer cmdBuf = m_commandBuffers[m_currentFrame];
	prog.Dispatch(cmdBuf, m_currentFrame, x, y, z);
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
	return *m_textures[handle.id];
}

VulkanMesh& RendererVulkan::GetMeshEntry(MeshHandle handle) {
	return *m_meshes[handle.id];
}

VulkanGraphicsProgram& RendererVulkan::GetMaterialEntry(MaterialHandle handle) {
	return *m_materials[handle.id];
}

VulkanComputeProgram& RendererVulkan::GetComputeProgramEntry(ComputeProgramHandle handle) {
	return *m_computePrograms[handle.id];
}

VulkanBuffer& RendererVulkan::GetUniformBufferEntry(UniformBufferHandle handle) {
	return *m_uniformBuffers[handle.id];
}

VulkanBuffer& RendererVulkan::GetShaderStorageBufferEntry(ShaderStorageBufferHandle handle) {
	return *m_shaderStorageBuffers[handle.id];
}

VulkanFrameBuffer& RendererVulkan::GetFrameBufferEntry(FrameBufferHandle handle) {
	return *m_frameBuffers[handle.id];
}

VkImageView RendererVulkan::GetTextureImageView(TextureHandle handle) {
	VulkanTexture& texture = GetTextureEntry(handle);
	return texture.GetImageView();
}

VkSampler RendererVulkan::GetTextureSampler(TextureHandle handle) {
	VulkanTexture& texture = GetTextureEntry(handle);
	return texture.GetSampler();
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
	m_swapchain = std::make_unique<VulkanSwapchain>(
	    m_device,
	    VkExtent2D{ m_surfaceWidth, m_surfaceHeight },
	    m_renderPass
	);
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
