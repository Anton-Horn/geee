#pragma once
#include <core/core.h>
#include "vulkan_core.h"

namespace ec {

	class VulkanSynchronisationController {

	public:

		void create(VulkanContext& context);
		void destroy(VulkanContext& context);

		void waitAndBeginFrame(VulkanContext& context, VulkanWindow& window);
		void submitFrameAndPresent(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data);
		void waitDeviceIdle(VulkanContext& context);

	private:

		VkFence m_fence;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

	};

}