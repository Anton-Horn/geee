#pragma once
#include <core/core.h>
#include "vulkan_core.h"

namespace ec {

	class VulkanSynchronisationController {

	public:

		void create(VulkanContext& context);
		void destroy(VulkanContext& context);

		void waitAndBeginFrame(VulkanContext& context, VulkanWindow& window, bool& recreateSwapchain);
		void submitFrameAndPresent(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data, bool& recreateSwapchain);
		void waitDeviceIdle(const VulkanContext& context) const;

	private:

		VkFence m_fence;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

	};

}