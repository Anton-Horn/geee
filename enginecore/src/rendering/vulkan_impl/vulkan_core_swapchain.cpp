#include "vulkan_core.h"

namespace ec {

	void VulkanSwapchain::create(VulkanContext& context, VkSurfaceKHR surface)
	{

		VkBool32 supportsPresent = false;
		VKA(vkGetPhysicalDeviceSurfaceSupportKHR(context.getData().physicalDevice, context.getData().queueFamilyIndex, surface, &supportsPresent));

		assert(supportsPresent);

		uint32_t formatsCount = 0;
		VKA(vkGetPhysicalDeviceSurfaceFormatsKHR(context.getData().physicalDevice, surface, &formatsCount, nullptr));
		VkSurfaceFormatKHR* surfaceFormats = new VkSurfaceFormatKHR[formatsCount];
		VKA(vkGetPhysicalDeviceSurfaceFormatsKHR(context.getData().physicalDevice, surface, &formatsCount, surfaceFormats));

		if (formatsCount <= 0) {
			EC_ERROR("No surface formats available");
			delete[] surfaceFormats;
			assert(false);
		}

		VkFormat format = surfaceFormats[0].format;
		VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

		delete[] surfaceFormats;

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VKA(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.getData().physicalDevice, surface, &surfaceCapabilities));

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

		VKA(vkCreateSwapchainKHR(context.getData().device, &swapchainCreateInfo, nullptr, &m_swapchain));

		m_format = format;
		m_swapchainWidth = surfaceCapabilities.currentExtent.width;
		m_swapchainHeight = surfaceCapabilities.currentExtent.height;


		uint32_t imageCount;
		VKA(vkGetSwapchainImagesKHR(context.getData().device, m_swapchain, &imageCount, nullptr));

		std::vector<VkImage> vkImages;
		vkImages.resize(imageCount);

		VKA(vkGetSwapchainImagesKHR(context.getData().device, m_swapchain, &imageCount, vkImages.data()));

		std::vector<VkImageView> vkImageViews;

		vkImageViews.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			createInfo.image = vkImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
			createInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1 };
			VKA(vkCreateImageView(context.getData().device, &createInfo, nullptr, &vkImageViews[i]));
		}

		m_images.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; i++) {
			m_images[i].create(vkImages[i], vkImageViews[i], m_swapchainWidth, m_swapchainHeight);
		}

	}

	void VulkanSwapchain::recreate(VulkanContext& context, VkSurfaceKHR surface) {

		destroy(context);
		create(context, surface);

	}

	void VulkanSwapchain::destroy(VulkanContext& context)
	{
		for (auto& image : m_images) {
			vkDestroyImageView(context.getData().device, image.getImageView(), nullptr);
		}

		vkDestroySwapchainKHR(context.getData().device, m_swapchain, nullptr);
	}

	VkResult VulkanSwapchain::aquireNextImage(VulkanContext& context, VkSemaphore signalSemaphore)
	{
		return vkAcquireNextImageKHR(context.getData().device, m_swapchain, UINT64_MAX, signalSemaphore, 0, &m_currentSwapchainImageIndex);
	}

	uint32_t VulkanSwapchain::getWidth() const
	{
		return m_swapchainWidth;
	}

	uint32_t VulkanSwapchain::getHeight() const
	{
		return m_swapchainHeight;
	}

	uint32_t VulkanSwapchain::getCurrentIndex() const
	{
		return m_currentSwapchainImageIndex;
	}

	VkFormat VulkanSwapchain::getFormat() const
	{
		return m_format;
	}

	const std::vector<VulkanImage>& VulkanSwapchain::getImages() const
	{
		return m_images;
	}

	const VkSwapchainKHR VulkanSwapchain::getSwapchain() const
	{
		return m_swapchain;
	}

}
