#pragma once
#include "PixieRendering/Renderer/Renderer.h"

#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class RendererVulkan : public Renderer {
public:
	RendererVulkan();
	~RendererVulkan();

	void StartFrame();
	void EndFrame();

	ShaderHandle CreateShader(const std::string& vertexShaderSource, const std::string fragmentShaderShource);
	MeshHandle LoadMesh(const Mesh* mesh);
	TextureHandle LoadTexture(float* data, glm::ivec2 resolution);
	TextureHandle LoadTexture(glm::vec3* data, glm::ivec2 resolution);
	TextureHandle LoadTexture(glm::vec4* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, float* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, glm::vec3* data, glm::ivec2 resolution);
	void LoadTexture(TextureHandle handle, glm::vec4* data, glm::ivec2 resolution);

	void CheckAvailableExtensions();
	bool CheckValidationLayerSupport();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule CreateShaderModule(const std::string& source, const std::string& fileName, uint32_t stage);
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void CreateSyncObjects();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


	VkApplicationInfo m_vkAppInfo;
	VkInstance m_vkInstance;
	VkPhysicalDevice m_vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_vkDevice;
	VkSurfaceKHR m_vkSurface;
	VkQueue m_vkPresentQueue;
	VkSwapchainKHR m_vkSwapChain;
	std::vector<VkImage> m_vkSwapChainImages;
	VkFormat m_vkSwapChainImageFormat;
	VkExtent2D m_vkSwapChainExtent;
	std::vector<VkImageView> m_vkSwapChainImageViews;
	VkPipelineLayout m_vkPipelineLayout;
	VkRenderPass m_vkRenderPass;
	VkPipeline m_vkPipeline;
	std::vector<VkFramebuffer> m_vkSwapChainFramebuffers;
	VkCommandPool m_vkCommandPool;
	VkCommandBuffer m_vkCommandBuffer;
	VkSemaphore m_vkImageAvailableSemaphore;
	VkSemaphore m_vkRenderFinishedSemaphore;
	VkFence m_vkInFlightFence;
};
