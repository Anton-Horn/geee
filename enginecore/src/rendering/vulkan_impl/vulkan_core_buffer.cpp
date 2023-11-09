#include "vulkan_core.h"

namespace ec {

	void VulkanBuffer::create(VulkanContext& context, uint64_t size, VkBufferUsageFlags usage, MemoryType type)
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

		VKA(vmaCreateBuffer(context.getData().allocator, &createInfo, &allocationCreateInfo, &m_buffer, &m_allocation, nullptr));
	}

	void VulkanBuffer::destroy(VulkanContext& context)
	{
		vmaDestroyBuffer(context.getData().allocator, m_buffer, m_allocation);
	}

	void VulkanBuffer::uploadData(VulkanContext& context, void* data, uint32_t size, uint32_t offset)
	{

		assert(offset + size <= getSize(context));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		VkMemoryType type = context.getData().deviceMemoryProperties.memoryTypes[allocationInfo.memoryType];

		if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {

			void* dstPointer;
			VKA(vmaMapMemory(context.getData().allocator, m_allocation, &dstPointer));
			dstPointer = ((uint8_t*)dstPointer) + offset;
			memcpy(dstPointer, data, size);
			vmaUnmapMemory(context.getData().allocator, m_allocation);

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
			VKA(vmaCreateBuffer(context.getData().allocator, &createInfo, &allocationCreateInfo, &stagingBuffer, &stagingAllocation, nullptr));

			VmaAllocationInfo stagingInfo;
			vmaGetAllocationInfo(context.getData().allocator, stagingAllocation, &stagingInfo);

			//Copy data into staging Buffer
			void* dstPointer = nullptr;
			VKA(vmaMapMemory(context.getData().allocator, stagingAllocation, &dstPointer));
			memcpy(dstPointer, data, size);
			vmaUnmapMemory(context.getData().allocator, stagingAllocation);

			//Create Command buffers

			VkCommandPool commandPool;
			VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			commandPoolCreateInfo.queueFamilyIndex = context.getData().queueFamilyIndex;
			commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			VKA(vkCreateCommandPool(context.getData().device, &commandPoolCreateInfo, nullptr, &commandPool));

			VkCommandBuffer commandBuffer;
			VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			allocateInfo.commandBufferCount = 1;
			allocateInfo.commandPool = commandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			VKA(vkAllocateCommandBuffers(context.getData().device, &allocateInfo, &commandBuffer));

			VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkBufferCopy copy;
			copy.dstOffset = offset;
			copy.srcOffset = 0;
			copy.size = size;
			vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_buffer, 1, &copy);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(context.getData().queue, 1, &submitInfo, 0);

			vkQueueWaitIdle(context.getData().queue);

			vkDestroyCommandPool(context.getData().device, commandPool, nullptr);
			vmaDestroyBuffer(context.getData().allocator, stagingBuffer, stagingAllocation);

		}
	}

	uint64_t VulkanBuffer::getSize(VulkanContext& context)
	{

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(context.getData().allocator, m_allocation, &allocationInfo);

		return allocationInfo.size;
	}

	const VkBuffer VulkanBuffer::getBuffer() const
	{
		return m_buffer;
	}

}