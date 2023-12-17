#pragma once

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

	};
}