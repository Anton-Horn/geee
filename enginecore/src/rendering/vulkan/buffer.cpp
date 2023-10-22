#include "vulkan_core.h"

namespace ec {

	void createVulkanBuffer(VulkanContext& context, VulkanBuffer& buffer, uint64_t size, VkBufferUsageFlags usage, MemoryType type)
	{

		VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.size = size;
		createInfo.usage = usage;

		VmaAllocationCreateInfo allocationCreateInfo = {};

		if (type == MemoryType::Auto) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}
		else if (type == MemoryType::Device_local) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (type == MemoryType::Host_local) {
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		}
		allocationCreateInfo.flags = 0;

		VKA(vmaCreateBuffer(context.allocator, &createInfo, &allocationCreateInfo, &buffer.buffer, &buffer.allocation, nullptr));
	}

	void destroyVulkanBuffer(VulkanContext& context, VulkanBuffer& buffer)
	{
		vmaDestroyBuffer(context.allocator, buffer.buffer, buffer.allocation);
	}

	void vulkanBufferUploadData(VulkanContext& context, VulkanBuffer& buffer, void* data, uint32_t size, uint32_t offset)
	{

		assert(offset + size <= vulkanBufferGetSize(context, buffer));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.allocator, buffer.allocation, &allocationInfo);

		VkMemoryType type = context.deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {

			void* dstPointer;
			VKA(vmaMapMemory(context.allocator, buffer.allocation, &dstPointer));
			dstPointer = ((uint8_t*)dstPointer) + offset;
			memcpy(dstPointer, data, size);
			vmaUnmapMemory(context.allocator, buffer.allocation);

		}
		else {



			//Staging Buffer

			VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.size = size;
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
			memcpy(dstPointer, data, size);
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

			VkBufferCopy copy;
			copy.dstOffset = offset;
			copy.srcOffset = 0;
			copy.size = size;
			vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer.buffer, 1, &copy);

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

	uint64_t vulkanBufferGetSize(VulkanContext& context, VulkanBuffer& buffer)
	{

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.allocator, buffer.allocation, &allocationInfo);

		return allocationInfo.size;
	}

}