#include "vulkan_synchronisation.h"
#include "vulkan_utils.h"


namespace ec {

	void VulkanSynchronisationController::create(const VulkanContext& context)
	{

		m_fences.resize(1);

		m_fences[0] = createFence(context);
		m_aquireSemaphore = createSemaphore(context);
		m_submitSemaphore = createSemaphore(context);

	}

	void VulkanSynchronisationController::destroy(const VulkanContext& context) {

		for (VkFence fence : m_fences) {
			vkDestroyFence(context.getData().device, fence, nullptr);
		}
		
		m_fences.clear();

		vkDestroySemaphore(context.getData().device, m_aquireSemaphore, nullptr);
		vkDestroySemaphore(context.getData().device, m_submitSemaphore, nullptr);

	}


	void VulkanSynchronisationController::waitAndAquireImage(const VulkanContext& context, VulkanWindow& window, bool& recreateSwapchain, bool signalAquireSemaphore) {

		uint32_t imageIndex = 0;

		VKA(vkWaitForFences(context.getData().device, m_fences.size(), m_fences.data(), true, UINT64_MAX));
		VKA(vkResetFences(context.getData().device, m_fences.size(), m_fences.data()));

		window.swapchain.aquireNextImage(context, m_aquireSemaphore, recreateSwapchain);

	}

	void VulkanSynchronisationController::wait(const VulkanContext& context)
	{

		uint32_t imageIndex = 0;

		VKA(vkWaitForFences(context.getData().device, m_fences.size(), m_fences.data(), true, UINT64_MAX));
		VKA(vkResetFences(context.getData().device, m_fences.size(), m_fences.data()));

	}

	void VulkanSynchronisationController::submitFrame(const VulkanContext& context, const std::vector<VkCommandBuffer>& data, bool waitForSwapchainImage, VkFence fence, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore) {

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = (uint32_t)data.size();
		submitInfo.pCommandBuffers = data.data();
		
		submitInfo.signalSemaphoreCount = 1;
		if (!signalSemaphore) signalSemaphore = m_submitSemaphore;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		if (waitForSwapchainImage) {
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &m_aquireSemaphore;
		}
		else {
			if (waitSemaphore) {
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = &waitSemaphore;
			}
		}

		VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitMask;
		
		if (!fence) fence = m_fences[0];

		std::lock_guard<std::mutex> lock(m_submitMutex);
		VKA(vkQueueSubmit(context.getData().queue, 1, &submitInfo, fence));

	}

	void VulkanSynchronisationController::waitDeviceIdle(const VulkanContext& context) const {

		vkDeviceWaitIdle(context.getData().device);

	}

	VkFence VulkanSynchronisationController::addFence(const VulkanContext& context)
	{
		VkFence r = createFence(context);
		m_fences.push_back(r);
		return r;
	}

	VkSemaphore VulkanSynchronisationController::getSubmitSemaphore()
	{
		return m_submitSemaphore;
	}

	VkSemaphore VulkanSynchronisationController::getAquireSemaphore()
	{
		return m_aquireSemaphore;
	}

	const VkSemaphore* VulkanSynchronisationController::getAquireSemaphorePtr()
	{
		return &m_aquireSemaphore;
	}

}