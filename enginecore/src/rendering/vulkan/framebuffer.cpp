#include "vulkan_core.h"

namespace ec {

	void VulkanFramebuffer::create(VulkanContext& context, VulkanRenderpass& renderpass, const std::vector<VulkanImage>& images) {

		EC_ASSERT(images.size());

		std::vector<VkImageView> imageViews;
		imageViews.resize(images.size());

		for (uint32_t i = 0; i < images.size(); i++) {
			imageViews[i] = images[i].imageView;
		}

		VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		createInfo.width = images[0].imageWidth;
		createInfo.height = images[0].imageHeight;
		createInfo.renderPass = renderpass.renderpass;
		createInfo.attachmentCount = imageViews.size();
		createInfo.pAttachments = imageViews.data();
		createInfo.layers = 1;

		vkCreateFramebuffer(context.device, &createInfo, nullptr, &framebuffer);

	}

	void VulkanFramebuffer::destroy(VulkanContext& context)
	{
		vkDestroyFramebuffer(context.device, framebuffer, nullptr);
	}

}
