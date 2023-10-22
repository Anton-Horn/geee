#include "vulkan_core.h"

namespace ec {

	void createVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain, VkSurfaceKHR surface)
	{

		VkBool32 supportsPresent = false;
		VKA(vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, context.queueFamilyIndex, surface, &supportsPresent));

		assert(supportsPresent);

		uint32_t formatsCount = 0;
		VKA(vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, surface, &formatsCount, nullptr));
		VkSurfaceFormatKHR* surfaceFormats = new VkSurfaceFormatKHR[formatsCount];
		VKA(vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, surface, &formatsCount, surfaceFormats));

		if (formatsCount <= 0) {
			EC_ERROR("No surface formats available");
			delete[] surfaceFormats;
			assert(false);
		}

		VkFormat format = surfaceFormats[0].format;
		VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

		delete[] surfaceFormats;

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VKA(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice, surface, &surfaceCapabilities));

		if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
			surfaceCapabilities.currentExtent.width = surfaceCapabilities.minImageExtent.width;
		}

		if (surfaceCapabilities.currentExtent.height == 0xFFFFFFFF) {
			surfaceCapabilities.currentExtent.height = surfaceCapabilities.minImageExtent.height;
		}

		if (surfaceCapabilities.maxImageCount == 0) {
			surfaceCapabilities.maxImageCount = 8;
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.minImageCount = 3;
		swapchainCreateInfo.imageFormat = format;
		swapchainCreateInfo.imageColorSpace = colorSpace;
		swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchainCreateInfo.clipped = VK_FALSE;

		VKA(vkCreateSwapchainKHR(context.device, &swapchainCreateInfo, nullptr, &swapchain.swapchain));

		swapchain.format = format;
		swapchain.swapchainWidth = surfaceCapabilities.currentExtent.width;
		swapchain.swapchainHeight = surfaceCapabilities.currentExtent.height;


		uint32_t imageCount;
		VKA(vkGetSwapchainImagesKHR(context.device, swapchain.swapchain, &imageCount, nullptr));

		std::vector<VkImage> vkImages;
		vkImages.resize(imageCount);

		VKA(vkGetSwapchainImagesKHR(context.device, swapchain.swapchain, &imageCount, vkImages.data()));

		std::vector<VkImageView> vkImageViews;

		vkImageViews.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			createInfo.image = vkImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
			createInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1 };
			VKA(vkCreateImageView(context.device, &createInfo, nullptr, &vkImageViews[i]));
		}

		swapchain.images.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; i++) {
			createVulkanImage(swapchain.images[i], vkImages[i], vkImageViews[i]);
			swapchain.images[i].imageWidth = swapchain.swapchainWidth;
			swapchain.images[i].imageHeight = swapchain.swapchainHeight;
		}

	}

	void reCreateVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain, VkSurfaceKHR surface) {

		destroyVulkanSwapchain(context, swapchain);
		createVulkanSwapchain(context, swapchain, surface);

	}

	void destroyVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain)
	{
		for (auto& image : swapchain.images) {
			vkDestroyImageView(context.device, image.imageView, nullptr);
		}

		vkDestroySwapchainKHR(context.device, swapchain.swapchain, nullptr);
	}

	void vulkanSwapchainAquireNextImage(VulkanContext& context, VulkanSwapchain& swapchain, VkSemaphore signalSemaphore)
	{
		VKA(vkAcquireNextImageKHR(context.device, swapchain.swapchain, UINT64_MAX, signalSemaphore, 0, &swapchain.currentSwapchainImageIndex));
	}

}