#pragma once
#include <core/core.h>
#include "vulkan_core.h"

namespace ec {

	class VulkanSynchronisationController {

	public:

		void create(const VulkanContext& context);
		void destroy(const VulkanContext& context);
		
		void waitAndAquireImage(const VulkanContext& context, VulkanWindow& window, bool& recreateSwapchain);
		void submitFrame(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data);
		void waitDeviceIdle(const VulkanContext& context) const;

		VkSemaphore getSubmitSemaphore();

	private:

		VkFence m_fence;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

	};

}