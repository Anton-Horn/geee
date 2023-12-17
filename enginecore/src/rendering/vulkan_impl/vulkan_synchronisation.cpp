#include "vulkan_synchronisation.h"
#include "vulkan_utils.h"


namespace ec {

	void VulkanSynchronisationController::create(const VulkanContext& context)
	{

		m_aquireSemaphore = createSemaphore(context);
		m_submitSemaphore = createSemaphore(context);
		m_fence = createFence(context);

	}

	void VulkanSynchronisationController::destroy(const VulkanContext& context) {

		vkDestroyFence(context.getData().device,m_fence, nullptr);
		vkDestroySemaphore(context.getData().device, m_aquireSemaphore, nullptr);
		vkDestroySemaphore(context.getData().device, m_submitSemaphore, nullptr);

	}


	void VulkanSynchronisationController::waitAndAquireImage(const VulkanContext& context, VulkanWindow& window, bool& recreateSwapchain) {

		uint32_t imageIndex = 0;

		VKA(vkWaitForFences(context.getData().device, 1, &m_fence, true, UINT64_MAX));

		VKA(vkResetFences(context.getData().device, 1, &m_fence));

		recreateSwapchain = window.swapchain.aquireNextImage(context, m_aquireSemaphore);

	}

	void VulkanSynchronisationController::submitFrame(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data) {

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = (uint32_t)data.size();
		submitInfo.pCommandBuffers = data.data();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_aquireSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_submitSemaphore;

		VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitMask;

		VKA(vkQueueSubmit(context.getData().queue, 1, &submitInfo, m_fence));
	}

	void VulkanSynchronisationController::waitDeviceIdle(const VulkanContext& context) const {

		vkDeviceWaitIdle(context.getData().device);

	}

	VkSemaphore VulkanSynchronisationController::getSubmitSemaphore()
	{
		return m_submitSemaphore;
	}

}