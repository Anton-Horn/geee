#include <vulkan/vulkan.h>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"


namespace ec {

	void VulkanSynchronisationController::create(VulkanContext& context)
	{

		m_aquireSemaphore = createSemaphore(context);
		m_submitSemaphore = createSemaphore(context);
		m_fence = createFence(context);

	}

	void VulkanSynchronisationController::destroy(VulkanContext& context) {

		vkDestroyFence(context.getData().device,m_fence, nullptr);
		vkDestroySemaphore(context.getData().device, m_aquireSemaphore, nullptr);
		vkDestroySemaphore(context.getData().device, m_submitSemaphore, nullptr);

	}


	void VulkanSynchronisationController::waitAndBeginFrame(VulkanContext& context, VulkanWindow& window) {

		uint32_t imageIndex = 0;

		VKA(vkWaitForFences(context.getData().device, 1, &m_fence, true, UINT64_MAX));

		VKA(vkResetFences(context.getData().device, 1, &m_fence));

		window.swapchain.aquireNextImage(context, m_aquireSemaphore);

	}

	void VulkanSynchronisationController::submitFrameAndPresent(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data) {

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = data.size();
		submitInfo.pCommandBuffers = data.data();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_aquireSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_submitSemaphore;
		VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitMask;

		VKA(vkQueueSubmit(context.getData().queue, 1, &submitInfo, m_fence));

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.swapchainCount = 1;

		const VkSwapchainKHR swapchain = window.swapchain.getSwapchain();

		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_submitSemaphore;

		uint32_t currentIndex = window.swapchain.getCurrentIndex();

		presentInfo.pImageIndices = &currentIndex;

		VKA(vkQueuePresentKHR(context.getData().queue, &presentInfo));

	}

	void VulkanSynchronisationController::waitDeviceIdle(VulkanContext& context) {

		vkDeviceWaitIdle(context.getData().device);

	}

}