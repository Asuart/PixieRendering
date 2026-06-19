#pragma once
#include "MainWindow.h"

#include <vulkan/vulkan.hpp>

class MainWindowVulkan : public MainWindow {
public:
	MainWindowVulkan(const std::string& name, glm::ivec2 resolution) : MainWindow(name, resolution, RenderAPI::Vulkan) {}
	//~MainWindowVulkan();

	VkInstance GetInstance() { return m_vkInstance; }
	VkDevice GetDevice() { return m_vkDevice; }
	VkPhysicalDevice GetPhysicalDevice() { return m_vkPhysicalDevice; }

	VkQueue GetQueue() { return m_vkQueue; }
	VkRenderPass GetRenderPass() { return m_vkRenderPass; }
	VkCommandBuffer GetCommandBuffer() { return m_vkCommandBuffer; }

protected:
	uint32_t m_imageIndex;

	VkInstance m_vkInstance;
	VkDevice m_vkDevice;
	VkPhysicalDevice m_vkPhysicalDevice;

	VkQueue m_vkQueue;
	VkRenderPass m_vkRenderPass;
	VkCommandBuffer m_vkCommandBuffer;

	void InitSDL() {}
};

