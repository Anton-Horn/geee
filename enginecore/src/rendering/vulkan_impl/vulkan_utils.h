#pragma once
#include "vulkan_core.h"
#include "core/application.h"

namespace ec {

    VkDescriptorPool createDesciptorPool(VulkanContext& context, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t flags = 0);
    VkDescriptorSetLayout createSetLayout(VulkanContext& context, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VkDescriptorSet allocateDescriptorSet(VulkanContext& context, VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void writeDescriptorUniformBuffer(VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, VulkanBuffer& buffer, bool dynamic = false, uint32_t offset = 0, uint32_t range = 0);
    void writeCombinedImageSampler(VulkanContext& context, VkDescriptorSet descriptorSet, uint32_t binding, const VulkanImage& image, VkSampler sampler);

    VkAttachmentDescription createColorAttachment(uint32_t sampleCount);
    VkAttachmentDescription createDepthAttachment(uint32_t sampleCount);
    VkAttachmentDescription createAttachment(uint32_t sampleCount, VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
    VkSubpassDescription createSubpass(std::vector<VkAttachmentReference>& colorAttachments, std::vector<VkAttachmentReference>& resolveAttachments, std::vector<VkAttachmentReference>& inputAttachments, std::optional<VkAttachmentReference> depthStencilAttachment = std::nullopt);

    VkCommandPool createCommandPool(VulkanContext& context);
    VkCommandBuffer allocateCommandBuffer(VulkanContext& context, VkCommandPool commandPool);
    VkFence createFence(VulkanContext& context);
    VkSemaphore createSemaphore(VulkanContext& context);
    VkSurfaceKHR createSurface(VulkanContext& context, const Window& window);
    void destroySurface(VulkanContext& context, VkSurfaceKHR surface);

    uint32_t alignToPow2(uint32_t alignment, uint32_t value);
    VkSampleCountFlagBits getSampleCount(uint8_t sampleCount);
    uint32_t getFormatSize(VkFormat format);
    std::vector<const char*> getInstanceExtensions();

}
