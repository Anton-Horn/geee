#include "vulkan_core.h"

namespace ec {

	void VulkanRenderpass::create(VulkanContext& context, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies) {

		VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.dependencyCount = dependencies.size();
		createInfo.pDependencies = dependencies.data();
		createInfo.subpassCount = subpasses.size();
		createInfo.pSubpasses = subpasses.data();

		vkCreateRenderPass(context.device, &createInfo, nullptr, &renderpass);

	}

	void VulkanRenderpass::destroy(VulkanContext& context) {



		vkDestroyRenderPass(context.device, renderpass, nullptr);

	}

}

