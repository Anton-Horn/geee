#pragma once
#include "vulkan_core.h"
#include "core/application.h"
namespace ec {

	static VkDescriptorPool createDesciptorPool(VulkanContext& context, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t flags = 0) {

		VkDescriptorPool result = 0;

		VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.poolSizeCount = poolSizes.size();
		createInfo.maxSets = maxSets;
		createInfo.flags = (VkDescriptorPoolCreateFlags)flags;

		VKA(vkCreateDescriptorPool(context.device, &createInfo, nullptr, &result));

		return result;
	}

	static VkDescriptorSetLayout createSetLayout(VulkanContext& context, const std::vector<VkDescriptorSetLayoutBinding>& bindings) {

		VkDescriptorSetLayout result;

		VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VKA(vkCreateDescriptorSetLayout(context.device, &createInfo, nullptr, &result));

		return result;
	}

	static VkDescriptorSet allocateDescriptorSet(VulkanContext& context, VkDescriptorPool pool, VkDescriptorSetLayout layout) {

		VkDescriptorSet result;

		VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorPool = pool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &layout;

		VKA(vkAllocateDescriptorSets(context.device, &allocateInfo, &result));

		return result;
	}

	static void writeDescriptorUniformBuffer(VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, VulkanBuffer& buffer, bool dynamic = false, uint32_t offset = 0, uint32_t range = 0) {

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = buffer.buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = dynamic ? range : vulkanBufferGetSize(context, buffer);

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.descriptorType = dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.dstBinding = binding;
		write.dstSet = descriptorSet;
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

	}

	static void writeCombinedImageSampler(VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, VulkanImage& image, VkSampler sampler) {

		VkDescriptorImageInfo imageInfo;
		imageInfo.imageView = image.imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstBinding = binding;
		write.dstSet = descriptorSet;
		write.pImageInfo = &imageInfo;


		vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

	}

	static uint32_t getFormatSize(VkFormat format) {
		switch (format) {
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_SRGB:
			return 4;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
			return 16;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
			return 8;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_SRGB:
			return 3;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SRGB:
			return 1;
		case VK_FORMAT_R32G32B32_SFLOAT:
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
			return 12;
		default:
			EC_ASSERT(false);
			return 0;
		}
	}

	static VkSampleCountFlagBits getSampleCount(uint8_t sampleCount)
	{
		switch (sampleCount) {
		case 1: return VK_SAMPLE_COUNT_1_BIT;
		case 2: return VK_SAMPLE_COUNT_2_BIT;
		case 4: return VK_SAMPLE_COUNT_4_BIT;
		case 8: return VK_SAMPLE_COUNT_8_BIT;
		case 16: return VK_SAMPLE_COUNT_16_BIT;
		case 32: return VK_SAMPLE_COUNT_32_BIT;
		default:

			EC_ERROR("unknown sample count: {0}", sampleCount);
			EC_ASSERT(false);
			return VK_SAMPLE_COUNT_1_BIT;
		}
	}

	static VkAttachmentDescription createColorAttachment(uint32_t sampleCount)
	{
		VkAttachmentDescription result = {};
		result.format = VK_FORMAT_R8G8B8A8_UNORM;
		result.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		result.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		result.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		return result;
	}

	static VkAttachmentDescription createDepthAttachment(uint32_t sampleCount)
	{
		VkAttachmentDescription result = {};
		result.format = VK_FORMAT_D32_SFLOAT;
		result.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		result.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		result.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		return result;
	}

	static VkAttachmentDescription createAttachment(uint32_t sampleCount, VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
	{
		VkAttachmentDescription result = {};
		result.format = format;
		result.finalLayout = finalLayout;
		result.initialLayout = initialLayout;
		result.samples = getSampleCount(sampleCount);
		result.loadOp = loadOp;
		result.storeOp = storeOp;
		return result;
	}

	static VkSubpassDescription createSubpass(const std::vector<VkAttachmentReference>& colorAttachments, std::optional<VkAttachmentReference> depthStencilAttachment = std::nullopt, const std::vector<VkAttachmentReference>& resolveAttachments = {}, const std::vector<VkAttachmentReference>& inputAttachments = {}) {


		VkSubpassDescription description = {};
		description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		description.colorAttachmentCount = colorAttachments.size();
		description.pColorAttachments = colorAttachments.data();

		description.pDepthStencilAttachment = depthStencilAttachment.has_value() ? &depthStencilAttachment.value() : nullptr;
		description.pResolveAttachments = resolveAttachments.data();

		description.inputAttachmentCount = inputAttachments.size();
		description.pInputAttachments = inputAttachments.data();

		return description;

	}

	static VkCommandPool createCommandPool(VulkanContext& context) {

		VkCommandPool result = {};

		VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		createInfo.queueFamilyIndex = context.queueFamilyIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		vkCreateCommandPool(context.device, &createInfo, nullptr, &result);
		return result;

	}

	static VkCommandBuffer allocateCommandBuffer(VulkanContext& context, VkCommandPool commandPool) {

		VkCommandBuffer result = {};

		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(context.device, &allocateInfo, &result);

		return result;

	}

	static VkFence createFence(VulkanContext& context) {

		VkFence result = {};

		VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(context.device, &createInfo, nullptr, &result);
		return result;
	}

	static VkSemaphore createSemaphore(VulkanContext& context) {

		VkSemaphore result = {};

		VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		vkCreateSemaphore(context.device, &createInfo, nullptr, &result);
		return result;
	}

	static uint32_t alignToPow2(uint32_t alignment, uint32_t value) {
		return  (value + alignment - 1) & ~(alignment - 1);
	}

	static VkSurfaceKHR getSurface(VulkanContext& context, Window& window) {

		VkSurfaceKHR surface;
		glfwCreateWindowSurface(context.instance, window.nativWindow, nullptr, &surface);
		return surface;

	}

	static std::vector<const char*> getInstanceExtensions() {

		uint32_t glfwInstanceExtensionsCount = 0;
		glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

		const char** glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

		std::vector<const char*> enabledInstanceExtensions;
		enabledInstanceExtensions.resize(glfwInstanceExtensionsCount);

		for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++) {
			enabledInstanceExtensions[i] = glfwInstanceExtensions[i];
		}

		return enabledInstanceExtensions;

	}

}