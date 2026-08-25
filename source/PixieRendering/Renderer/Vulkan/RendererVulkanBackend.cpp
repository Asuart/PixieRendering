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
