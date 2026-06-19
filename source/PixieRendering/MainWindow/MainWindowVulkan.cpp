//#include "MainWindowVulkan.h"
//
//namespace {
//
//const std::vector<const char*> validationLayers = {
//	"VK_LAYER_KHRONOS_validation"
//};
//
//const std::vector<const char*> deviceExtensions = {
//	VK_KHR_SWAPCHAIN_EXTENSION_NAME
//};
//
//#ifdef NDEBUG
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif
//
//}
//
//
//MainWindowVulkan::MainWindowVulkan(const std::string& name, glm::ivec2 resolution) :
//	MainWindow(name, resolution, RenderAPI::Vulkan) {
//	InitSDL();
//
//	m_vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//	m_vkAppInfo.pNext = NULL;
//	m_vkAppInfo.pApplicationName = name.c_str();
//	m_vkAppInfo.applicationVersion = 0;
//	m_vkAppInfo.pEngineName = nullptr;
//	m_vkAppInfo.engineVersion = 0;
//	m_vkAppInfo.apiVersion = VK_API_VERSION_1_4;
//
//	uint32_t extensionsCount;
//	const char** extensionsNames = 0;
//	SDL_Vulkan_GetInstanceExtensions(m_window, &extensionsCount, NULL);
//	extensionsNames = new const char* [extensionsCount];
//	SDL_Vulkan_GetInstanceExtensions(m_window, &extensionsCount, extensionsNames);
//
//	VkInstanceCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//	createInfo.pApplicationInfo = &m_vkAppInfo;
//	createInfo.enabledExtensionCount = extensionsCount;
//	createInfo.ppEnabledExtensionNames = extensionsNames;
//
//	if (enableValidationLayers && !CheckValidationLayerSupport()) {
//		Log::Error("Could not enable validation layers for Vulkan");
//		exit(2);
//	}
//
//	if (enableValidationLayers) {
//		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//		createInfo.ppEnabledLayerNames = validationLayers.data();
//	}
//	else {
//		createInfo.enabledLayerCount = 0;
//	}
//
//	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
//		Log::Error("Failed to create Vulkan instance");
//		exit(1);
//	}
//
//	if (SDL_Vulkan_CreateSurface(m_window, m_vkInstance, &m_vkSurface) != SDL_TRUE) {
//		Log::Error("Failed to create Vulkan surface");
//		exit(5);
//	}
//
//	CheckAvailableExtensions();
//
//	uint32_t deviceCount = 0;
//	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
//
//	if (deviceCount == 0) {
//		Log::Error("Failed to find GPUs with Vulkan support.");
//		exit(3);
//	}
//
//	std::vector<VkPhysicalDevice> devices(deviceCount);
//	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());
//
//	for (const auto& device : devices) {
//		if (IsDeviceSuitable(device)) {
//			m_vkPhysicalDevice = device;
//			break;
//		}
//	}
//
//	if (m_vkPhysicalDevice == VK_NULL_HANDLE) {
//		Log::Error("Failed to find a suitable GPU.");
//		exit(4);
//	}
//
//	QueueFamilyIndices indices = FindQueueFamilies(m_vkPhysicalDevice);
//
//	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
//	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
//
//	float queuePriority = 1.0f;
//	for (uint32_t queueFamily : uniqueQueueFamilies) {
//		VkDeviceQueueCreateInfo queueCreateInfo{};
//		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//		queueCreateInfo.queueFamilyIndex = queueFamily;
//		queueCreateInfo.queueCount = 1;
//		queueCreateInfo.pQueuePriorities = &queuePriority;
//		queueCreateInfos.push_back(queueCreateInfo);
//	}
//
//	VkPhysicalDeviceFeatures deviceFeatures{};
//
//	VkDeviceCreateInfo deviceCreateInfo{};
//	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();;
//	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
//	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
//
//	if (enableValidationLayers) {
//		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
//	}
//	else {
//		deviceCreateInfo.enabledLayerCount = 0;
//	}
//
//	if (vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice) != VK_SUCCESS) {
//		Log::Error("failed to create logical device.");
//		exit(4);
//	}
//
//	vkGetDeviceQueue(m_vkDevice, indices.presentFamily.value(), 0, &m_vkPresentQueue);
//
//	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_vkPhysicalDevice);
//
//	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
//	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
//	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
//
//	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//		imageCount = swapChainSupport.capabilities.maxImageCount;
//	}
//
//	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
//	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//	swapchainCreateInfo.surface = m_vkSurface;
//	swapchainCreateInfo.minImageCount = imageCount;
//	swapchainCreateInfo.imageFormat = surfaceFormat.format;
//	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
//	swapchainCreateInfo.imageExtent = extent;
//	swapchainCreateInfo.imageArrayLayers = 1;
//	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//	swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//	swapchainCreateInfo.presentMode = presentMode;
//	swapchainCreateInfo.clipped = VK_TRUE;
//	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
//
//	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
//
//	if (indices.graphicsFamily != indices.presentFamily) {
//		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//		swapchainCreateInfo.queueFamilyIndexCount = 2;
//		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
//	}
//	else {
//		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//		swapchainCreateInfo.queueFamilyIndexCount = 0;
//		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
//	}
//
//	m_vkSwapChainImageFormat = surfaceFormat.format;
//	m_vkSwapChainExtent = extent;
//
//	if (vkCreateSwapchainKHR(m_vkDevice, &swapchainCreateInfo, nullptr, &m_vkSwapChain) != VK_SUCCESS) {
//		Log::Error("Failed to create Vulkan swap chain.");
//		exit(6);
//	}
//
//	vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, nullptr);
//	m_vkSwapChainImages.resize(imageCount);
//	vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, m_vkSwapChainImages.data());
//
//	m_vkSwapChainImageViews.resize(m_vkSwapChainImages.size());
//	for (size_t i = 0; i < m_vkSwapChainImages.size(); i++) {
//		VkImageViewCreateInfo imageViewCreateInfo{};
//		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		imageViewCreateInfo.image = m_vkSwapChainImages[i];
//		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		imageViewCreateInfo.format = m_vkSwapChainImageFormat;
//		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//		imageViewCreateInfo.subresourceRange.levelCount = 1;
//		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//		imageViewCreateInfo.subresourceRange.layerCount = 1;
//
//		if (vkCreateImageView(m_vkDevice, &imageViewCreateInfo, nullptr, &m_vkSwapChainImageViews[i]) != VK_SUCCESS) {
//			Log::Error("Failed to create Vulkan image views.");
//			exit(7);
//		}
//	}
//
//	CreateRenderPass();
//	CreateGraphicsPipeline();
//	CreateFramebuffers();
//	CreateCommandPool();
//	CreateCommandBuffer(); 
//	CreateSyncObjects();
//
//	std::cout << "Vulkan window created\n";
//}
//
//MainWindowVulkan::~MainWindowVulkan() {
//	vkDeviceWaitIdle(m_vkDevice);
//
//	vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphore, nullptr);
//	vkDestroySemaphore(m_vkDevice, m_vkRenderFinishedSemaphore, nullptr);
//	vkDestroyFence(m_vkDevice, m_vkInFlightFence, nullptr);
//	vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);
//	for (auto framebuffer : m_vkSwapChainFramebuffers) {
//		vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
//	}
//	vkDestroyPipeline(m_vkDevice, m_vkPipeline, nullptr);
//	vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, nullptr);
//	vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, nullptr);
//	for (auto imageView : m_vkSwapChainImageViews) {
//		vkDestroyImageView(m_vkDevice, imageView, nullptr);
//	}
//	vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
//	vkDestroyDevice(m_vkDevice, nullptr);
//	vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
//	vkDestroyInstance(m_vkInstance, nullptr);
//	SDL_DestroyWindow(m_window);
//	SDL_Quit();
//}
//
//void MainWindowVulkan::InitSDL() {
//	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
//		Log::Error("SDL2 initialization failed.");
//		exit(1);
//	}
//
//	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//
//	m_window = SDL_CreateWindow(m_name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_resolution.x, m_resolution.y, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
//	if (!m_window) {
//		Log::Error("SDL2 failed to create a Vulkan window.");
//		exit(2);
//	}
//}
//
//void MainWindowVulkan::StartFrame() {
//	vkWaitForFences(m_vkDevice, 1, &m_vkInFlightFence, VK_TRUE, UINT64_MAX);
//	vkResetFences(m_vkDevice, 1, &m_vkInFlightFence);
//
//	vkAcquireNextImageKHR(m_vkDevice, m_vkSwapChain, UINT64_MAX, m_vkImageAvailableSemaphore, VK_NULL_HANDLE, &m_imageIndex);
//
//	vkResetCommandBuffer(m_vkCommandBuffer, 0);
//
//	//RecordCommandBuffer(m_vkCommandBuffer, m_imageIndex);
//
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = 0;
//	beginInfo.pInheritanceInfo = nullptr;
//
//	if (vkBeginCommandBuffer(m_vkCommandBuffer, &beginInfo) != VK_SUCCESS) {
//		Log::Error("Failed to begin recording command buffer.");
//		exit(1);
//	}
//
//	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
//
//	VkRenderPassBeginInfo renderPassInfo{};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderPassInfo.renderPass = m_vkRenderPass;
//	renderPassInfo.framebuffer = m_vkSwapChainFramebuffers[m_imageIndex];
//	renderPassInfo.renderArea.offset = { 0, 0 };
//	renderPassInfo.renderArea.extent = m_vkSwapChainExtent;
//	renderPassInfo.clearValueCount = 1;
//	renderPassInfo.pClearValues = &clearColor;
//
//	vkCmdBeginRenderPass(m_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
//
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(m_vkSwapChainExtent.width);
//	viewport.height = static_cast<float>(m_vkSwapChainExtent.height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//	vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &viewport);
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = m_vkSwapChainExtent;
//	vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &scissor);
//
//	//vkCmdDraw(m_vkCommandBuffer, 3, 1, 0, 0);
//}
//
//void MainWindowVulkan::EndFrame() {
//	vkCmdEndRenderPass(m_vkCommandBuffer);
//
//	if (vkEndCommandBuffer(m_vkCommandBuffer) != VK_SUCCESS) {
//		Log::Error("Failed to record command buffer.");
//		exit(1);
//	}
//
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphore };
//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;
//	submitInfo.pWaitDstStageMask = waitStages;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &m_vkCommandBuffer;
//
//	VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphore };
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;
//
//	if (vkQueueSubmit(m_vkPresentQueue, 1, &submitInfo, m_vkInFlightFence) != VK_SUCCESS) {
//		Log::Error("Failed to submit draw command buffer.");
//	}
//
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = signalSemaphores;
//
//	VkSwapchainKHR swapChains[] = { m_vkSwapChain };
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = swapChains;
//	presentInfo.pImageIndices = &m_imageIndex;
//	presentInfo.pResults = nullptr;
//
//	vkQueuePresentKHR(m_vkPresentQueue, &presentInfo);
//}
//
//void MainWindowVulkan::CheckAvailableExtensions() {
//	uint32_t extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//
//	std::vector<VkExtensionProperties> extensions(extensionCount);
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
//
//	Log::Message("Available Extensions:");
//	for (const auto& extension : extensions) {
//		Log::Message(extension.extensionName);
//	}
//}
//
//bool MainWindowVulkan::CheckValidationLayerSupport() {
//	uint32_t layerCount;
//	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//
//	std::vector<VkLayerProperties> availableLayers(layerCount);
//	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//
//	for (const char* layerName : validationLayers) {
//		bool layerFound = false;
//
//		for (const auto& layerProperties : availableLayers) {
//			if (strcmp(layerName, layerProperties.layerName) == 0) {
//				layerFound = true;
//				break;
//			}
//		}
//
//		if (!layerFound) {
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool MainWindowVulkan::IsDeviceSuitable(VkPhysicalDevice device) {
//	VkPhysicalDeviceProperties deviceProperties;
//	vkGetPhysicalDeviceProperties(device, &deviceProperties);
//
//	VkPhysicalDeviceFeatures deviceFeatures;
//	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
//
//	QueueFamilyIndices indices = FindQueueFamilies(device);
//
//	bool extensionsSupported = CheckDeviceExtensionSupport(device);
//
//	bool swapChainAdequate = false;
//	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
//	swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//
//	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
//}
//
//QueueFamilyIndices MainWindowVulkan::FindQueueFamilies(VkPhysicalDevice device) {
//	QueueFamilyIndices indices;
//
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//
//	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
//
//	int32_t i = 0;
//	for (const auto& queueFamily : queueFamilies) {
//		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//			indices.graphicsFamily = i;
//		}
//
//		VkBool32 presentSupport = false;
//		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_vkSurface, &presentSupport);
//		if (presentSupport) {
//			indices.presentFamily = i;
//		}
//
//		if (indices.IsComplete()) {
//			break;
//		}
//
//		i++;
//	}
//
//	return indices;
//}
//
//bool MainWindowVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
//	return true;
//}
//
//SwapChainSupportDetails MainWindowVulkan::QuerySwapChainSupport(VkPhysicalDevice device) {
//	SwapChainSupportDetails details;
//
//	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_vkSurface, &details.capabilities);
//
//	uint32_t formatCount;
//	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurface, &formatCount, nullptr);
//
//	if (formatCount != 0) {
//		details.formats.resize(formatCount);
//		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurface, &formatCount, details.formats.data());
//	}
//
//	uint32_t presentModeCount;
//	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurface, &presentModeCount, nullptr);
//
//	if (presentModeCount != 0) {
//		details.presentModes.resize(presentModeCount);
//		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurface, &presentModeCount, details.presentModes.data());
//	}
//
//	return details;
//}
//
//VkSurfaceFormatKHR MainWindowVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
//	for (const auto& availableFormat : availableFormats) {
//		if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//			return availableFormat;
//		}
//	}
//
//	return availableFormats[0];
//}
//
//VkPresentModeKHR MainWindowVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
//	for (const auto& availablePresentMode : availablePresentModes) {
//		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//			return availablePresentMode;
//		}
//	}
//	return VK_PRESENT_MODE_FIFO_KHR;
//}
//
//VkExtent2D MainWindowVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
//	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//		return capabilities.currentExtent;
//	}
//	else {
//		VkExtent2D actualExtent = { m_resolution.x, m_resolution.y };
//
//		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
//
//		return actualExtent;
//	}
//}
//
//void MainWindowVulkan::CreateRenderPass() {
//	VkAttachmentDescription colorAttachment{};
//	colorAttachment.format = m_vkSwapChainImageFormat;
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//	VkAttachmentReference colorAttachmentRef{};
//	colorAttachmentRef.attachment = 0;
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpass{};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &colorAttachmentRef;
//
//	VkSubpassDependency dependency{};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependency.dstSubpass = 0;
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//	VkRenderPassCreateInfo renderPassInfo{};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderPassInfo.attachmentCount = 1;
//	renderPassInfo.pAttachments = &colorAttachment;
//	renderPassInfo.subpassCount = 1;
//	renderPassInfo.pSubpasses = &subpass;
//	renderPassInfo.dependencyCount = 1;
//	renderPassInfo.pDependencies = &dependency;
//
//	if (vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS) {
//		Log::Error("Failed to create render pass.");
//		exit(1);
//	}
//}
//
//void MainWindowVulkan::CreateGraphicsPipeline() {
//	const char* vertShaderSource = R"(
//		#version 450
//
//		layout(location = 0) out vec3 fragColor;
//		
//		vec2 positions[3] = vec2[](
//		    vec2(0.0, -0.5),
//		    vec2(0.5, 0.5),
//		    vec2(-0.5, 0.5)
//		);
//		
//		vec3 colors[3] = vec3[](
//		    vec3(1.0, 0.0, 0.0),
//		    vec3(0.0, 1.0, 0.0),
//		    vec3(0.0, 0.0, 1.0)
//		);
//		
//		void main() {
//		    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
//		    fragColor = colors[gl_VertexIndex];
//		}
//	)";
//
//	const char* fragShaderSource = R"(
//		#version 450
//
//		layout(location = 0) in vec3 fragColor;
//		
//		layout(location = 0) out vec4 outColor;
//		
//		void main() {
//		    outColor = vec4(fragColor, 1.0);
//		}
//	)";
//
//	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderSource, "Vertex Shader", GLSLANG_STAGE_VERTEX);
//	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderSource, "Fragment Shader", GLSLANG_STAGE_FRAGMENT);
//
//	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertShaderStageInfo.module = vertShaderModule;
//	vertShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragShaderStageInfo.module = fragShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//	std::vector<VkDynamicState> dynamicStates = {
//	VK_DYNAMIC_STATE_VIEWPORT,
//	VK_DYNAMIC_STATE_SCISSOR
//	};
//
//	VkPipelineDynamicStateCreateInfo dynamicState{};
//	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
//	dynamicState.pDynamicStates = dynamicStates.data();
//
//	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	vertexInputInfo.vertexBindingDescriptionCount = 0;
//	vertexInputInfo.pVertexBindingDescriptions = nullptr;
//	vertexInputInfo.vertexAttributeDescriptionCount = 0;
//	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
//
//	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = (float)m_vkSwapChainExtent.width;
//	viewport.height = (float)m_vkSwapChainExtent.height;
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = m_vkSwapChainExtent;
//
//	VkPipelineViewportStateCreateInfo viewportState{};
//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//
//	VkPipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterizer.depthClampEnable = VK_FALSE;
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;
//	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//	rasterizer.lineWidth = 1.0f;
//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//	rasterizer.depthBiasEnable = VK_FALSE;
//	rasterizer.depthBiasConstantFactor = 0.0f;
//	rasterizer.depthBiasClamp = 0.0f;
//	rasterizer.depthBiasSlopeFactor = 0.0f;
//
//	VkPipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//	multisampling.minSampleShading = 1.0f;
//	multisampling.pSampleMask = nullptr; 
//	multisampling.alphaToCoverageEnable = VK_FALSE;
//	multisampling.alphaToOneEnable = VK_FALSE;
//
//	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; 
//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
//
//	VkPipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = VK_LOGIC_OP_COPY;
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//	colorBlending.blendConstants[0] = 0.0f;
//	colorBlending.blendConstants[1] = 0.0f;
//	colorBlending.blendConstants[2] = 0.0f;
//	colorBlending.blendConstants[3] = 0.0f;
//
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipelineLayoutInfo.setLayoutCount = 0;
//	pipelineLayoutInfo.pSetLayouts = nullptr; 
//	pipelineLayoutInfo.pushConstantRangeCount = 0; 
//	pipelineLayoutInfo.pPushConstantRanges = nullptr; 
//
//	if (vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS) {
//		Log::Error("Failed to create pipeline layout.");
//		exit(1);
//	}
//
//	VkGraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//	pipelineInfo.stageCount = 2;
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pDepthStencilState = nullptr;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.pDynamicState = &dynamicState;
//	pipelineInfo.layout = m_vkPipelineLayout;
//	pipelineInfo.renderPass = m_vkRenderPass;
//	pipelineInfo.subpass = 0;
//	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//	pipelineInfo.basePipelineIndex = -1; 
//
//	if (vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkPipeline) != VK_SUCCESS) {
//		Log::Error("Failed to create graphics pipeline.");
//		exit(1);
//	}
//
//	vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);
//	vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
//}
//
//void MainWindowVulkan::CreateFramebuffers() {
//	m_vkSwapChainFramebuffers.resize(m_vkSwapChainImageViews.size());
//	for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++) {
//		VkImageView attachments[] = {
//			m_vkSwapChainImageViews[i]
//		};
//
//		VkFramebufferCreateInfo framebufferInfo{};
//		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		framebufferInfo.renderPass = m_vkRenderPass;
//		framebufferInfo.attachmentCount = 1;
//		framebufferInfo.pAttachments = attachments;
//		framebufferInfo.width = m_vkSwapChainExtent.width;
//		framebufferInfo.height = m_vkSwapChainExtent.height;
//		framebufferInfo.layers = 1;
//
//		if (vkCreateFramebuffer(m_vkDevice, &framebufferInfo, nullptr, &m_vkSwapChainFramebuffers[i]) != VK_SUCCESS) {
//			Log::Error("Failed to create framebuffer.");
//			exit(1);
//		}
//	}
//}
//
//void MainWindowVulkan::CreateCommandPool() {
//	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_vkPhysicalDevice);
//
//	VkCommandPoolCreateInfo poolInfo{};
//	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
//
//	if (vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_vkCommandPool) != VK_SUCCESS) {
//		Log::Error("Failed to create command pool.");
//		exit(1);
//	}
//}
//
//void MainWindowVulkan::CreateCommandBuffer() {
//	VkCommandBufferAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.commandPool = m_vkCommandPool;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandBufferCount = 1;
//
//	if (vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &m_vkCommandBuffer) != VK_SUCCESS) {
//		Log::Error("Failed to allocate command buffers.");
//		exit(1);
//	}
//}
//
//void MainWindowVulkan::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = 0;
//	beginInfo.pInheritanceInfo = nullptr; 
//
//	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
//		Log::Error("Failed to begin recording command buffer.");
//		exit(1);
//	}
//
//	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
//
//	VkRenderPassBeginInfo renderPassInfo{};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderPassInfo.renderPass = m_vkRenderPass;
//	renderPassInfo.framebuffer = m_vkSwapChainFramebuffers[imageIndex];
//	renderPassInfo.renderArea.offset = { 0, 0 };
//	renderPassInfo.renderArea.extent = m_vkSwapChainExtent;
//	renderPassInfo.clearValueCount = 1;
//	renderPassInfo.pClearValues = &clearColor;
//
//	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
//
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(m_vkSwapChainExtent.width);
//	viewport.height = static_cast<float>(m_vkSwapChainExtent.height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = m_vkSwapChainExtent;
//	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
//
//	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
//
//	vkCmdEndRenderPass(commandBuffer);
//
//	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
//		Log::Error("Failed to record command buffer.");
//		exit(1);
//	}
//}
//
//void MainWindowVulkan::CreateSyncObjects() {
//	VkSemaphoreCreateInfo semaphoreInfo{};
//	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	VkFenceCreateInfo fenceInfo{};
//	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//	if (vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_vkImageAvailableSemaphore) != VK_SUCCESS) {
//		Log::Error("Failed to create image available semaphore.");
//		exit(1);
//	}
//
//	if (vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_vkRenderFinishedSemaphore) != VK_SUCCESS) {
//		Log::Error("Failed to create render finished semaphore.");
//		exit(1);
//	}
//
//	if (vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &m_vkInFlightFence) != VK_SUCCESS) {
//		Log::Error("Failed to create in flight fence.");
//		exit(1);
//	}
//}
//