#pragma once
#include <core/core.h>
#include "vulkan_core.h"

namespace ec {


	class VulkanSynchronisationController {

	public:

		void create(const VulkanContext& context);
		void destroy(const VulkanContext& context);
		
		void waitAndAquireImage(const VulkanContext& context, VulkanWindow& window, bool& recreateSwapchain, bool signalAquireSemaphore = true);
		void wait(const VulkanContext& context);

		void submitFrame(const VulkanContext& context, const std::vector<VkCommandBuffer>& data, bool waitForSwapchainImage = true, VkFence fence = nullptr, VkSemaphore waitSemaphore = nullptr, VkSemaphore signalSemaphore = nullptr);
		void waitDeviceIdle(const VulkanContext& context) const;

		VkFence addFence(const VulkanContext& context);

		VkSemaphore getSubmitSemaphore();
		VkSemaphore getAquireSemaphore();
		const VkSemaphore* getAquireSemaphorePtr();

	private:

		std::vector<VkFence> m_fences;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

		std::mutex m_submitMutex;

		uint32_t m_flags;

	};

}