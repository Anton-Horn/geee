#include "vulkan_core.h"
#include "vulkan_utils.h"

namespace ec {

	void createVulkanImage(VulkanContext& context, VulkanImage& image, uint32_t width, uint32_t height, uint32_t format, uint32_t usageFlags, uint32_t sampleCount)
	{

		image.m_mutable = true;

		VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.format = (VkFormat)format;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.usage = usageFlags;
		createInfo.samples = getSampleCount(sampleCount);
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocationCreateInfo = {};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.priority = 1.0f;

		VKA(vmaCreateImage(context.allocator, &createInfo, &allocationCreateInfo, &image.image, &image.allocation, nullptr));

		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

		if (format == VK_FORMAT_D32_SFLOAT) {
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		{
			VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			createInfo.image = image.image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = (VkFormat)format;
			createInfo.subresourceRange.aspectMask = aspect;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.layerCount = 1;

			VKA(vkCreateImageView(context.device, &createInfo, nullptr, &image.imageView));
		}

		image.imageWidth = width;
		image.imageHeight = height;

	}

	void createVulkanImage(VulkanImage& image, VkImage vkImage, VkImageView view)
	{
		image.m_mutable = false;

		image.image = vkImage;
		image.imageView = view;
	}

	void destroyVulkanImage(VulkanContext& context, VulkanImage& image)
	{
		if (!image.m_mutable) return;
		vkDestroyImageView(context.device, image.imageView, nullptr);
		vmaDestroyImage(context.allocator, image.image, image.allocation);
	}

	void vulkanImageUploadData(VulkanContext& context, VulkanImage& image, void* data, uint32_t width, uint32_t height, uint32_t bytesPerPixel, uint32_t layout)
	{

		EC_ASSERT(image.m_mutable);
		//Staging Buffer

		VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.size = width * height * bytesPerPixel;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocationCreateInfo = {};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		//Create Buffer
		VKA(vmaCreateBuffer(context.allocator, &createInfo, &allocationCreateInfo, &stagingBuffer, &stagingAllocation, nullptr));

		VmaAllocationInfo stagingInfo;
		vmaGetAllocationInfo(context.allocator, stagingAllocation, &stagingInfo);

		//Copy data into staging Buffer
		void* dstPointer = nullptr;
		VKA(vmaMapMemory(context.allocator, stagingAllocation, &dstPointer));
		memcpy(dstPointer, data, width * height * bytesPerPixel);
		vmaUnmapMemory(context.allocator, stagingAllocation);

		//Create Command buffers

		VkCommandPool commandPool;
		VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		commandPoolCreateInfo.queueFamilyIndex = context.queueFamilyIndex;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		VKA(vkCreateCommandPool(context.device, &commandPoolCreateInfo, nullptr, &commandPool));

		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VKA(vkAllocateCommandBuffers(context.device, &allocateInfo, &commandBuffer));

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		{

			VkImageMemoryBarrier imageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.image = image.image;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.layerCount = 1;
			imageBarrier.srcAccessMask = 0;
			imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;


			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0, 1, &imageBarrier);

		}

		VkBufferImageCopy imageRegion = {};
		imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.imageSubresource.layerCount = 1;
		imageRegion.imageExtent = { width, height, 1 };
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

		{

			VkImageMemoryBarrier imageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;;
			imageBarrier.newLayout = (VkImageLayout)layout;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.image = image.image;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.layerCount = 1;
			imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_NONE;


			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0, 0, 0, 1, &imageBarrier);

		}

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(context.queue, 1, &submitInfo, 0);

		vkQueueWaitIdle(context.queue);

		vkDestroyCommandPool(context.device, commandPool, nullptr);
		vmaDestroyBuffer(context.allocator, stagingBuffer, stagingAllocation);

	}

}
