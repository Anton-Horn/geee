#pragma once

#include <vector>

#include "rendering/vulkan_impl/vulkan_core.h"
#include "rendering/vulkan_impl/vulkan_synchronisation.h"
#include "rendering/vulkan_impl/vulkan_renderer.h"


namespace ec {

	struct RendererData {

		VulkanContext context;
		VulkanWindow window;
		VulkanSynchronisationController synController;

		VulkanQuadRenderer quadRenderer;
		VulkanBezierRenderer bezierRenderer;
		VulkanGoochRenderer goochRenderer;
		VulkanMandelbrotRenderer mandelbrotRenderer;
		
		VulkanQuadRenderer presentRenderer;

		VulkanModel model;
		VulkanImage image;

		VkCommandPool commandPool;
		VkDescriptorPool rpfDescriptorPool;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> presentWaitSemaphores;

	};

	struct VulkanDataInterface {

		//only valid if RENDERER_PRESENT_TO_SWAPCHAIN is not set in create info
		std::function<const VkImage()> getPresentImageHandle;
		//only valid if RENDERER_PRESENT_TO_SWAPCHAIN is not set in create info
		std::function<const VkImageView()> getPresentImageView;

		//not thread safe, only called in a synchronized environment
		//should probably only be called in set up code.
		//prefer to submit empty command buffers than to add them later
		std::function<const VkCommandBuffer (VkCommandPool commandPool)> addCommandBuffer;
		std::function<const VkSemaphore ()> addPresentWaitSemaphore;
		std::function<const VkFence ()> addBeginFrameWaitingFence;

		//only valid if RENDERER_DONT_AQUIRE_IMAGE is set
		//should only be called once per frame
		//gpu synchronization and swapchain recreation needs to be handled by caller
		std::function<void(VkSemaphore signalSemaphore, bool& recreateSwapchain)> aquireNextSwapchainImage;

		//synchronizes all submits for the renderer queue
		//prefer to use this instead of vkQueueSubmit if same queue is being used
		//if waitForSwapchain = true then the other waitSemaphore is ignored
		std::function<void(const std::vector<VkCommandBuffer>& commandBuffers, bool waitForSwapchain, VkFence signalFence, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore)> submitFrameSynchronized;

		const VulkanWindow* window;
		const VulkanContext* context;

	};

}