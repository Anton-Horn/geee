#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "core/core.h"
#include "utils/file_utils.h"

struct SpvReflectDescriptorBinding;

namespace ec {

	struct VulkanContext {

		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkInstance instance;
		VkQueue queue;
		uint32_t queueFamilyIndex = 0;
		VkDebugUtilsMessengerEXT debugCallback;
		VmaAllocator allocator;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkDescriptorPool generalDescriptorPool;

	};

	void createDefaultVulkanContext(VulkanContext& context, const std::string& applicationName, std::vector<const char*>& additionalWindowInstanceExtensions);
	void createVulkanContext(VulkanContext& context, const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);
	void destroyVulkanContext(VulkanContext& context);

	enum class MemoryType {

		Device_local,
		Auto,
		Host_local

	};

	struct VulkanBuffer {

		VkBuffer buffer;
		VmaAllocation allocation;
		VkBufferUsageFlags usage;

	};

	void createVulkanBuffer(VulkanContext& context, VulkanBuffer& buffer, uint64_t size, uint32_t usage, MemoryType type = MemoryType::Auto);
	void destroyVulkanBuffer(VulkanContext& context, VulkanBuffer& buffer);
	void vulkanBufferUploadData(VulkanContext& context, VulkanBuffer& buffer, void* data, uint32_t size, uint32_t offset = 0);
	uint64_t vulkanBufferGetSize(VulkanContext& context, VulkanBuffer& buffer);

	struct VulkanImage {

		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		uint32_t imageWidth, imageHeight;

		bool m_mutable = false;

	};

	void createVulkanImage(VulkanContext& context, VulkanImage& image, uint32_t width, uint32_t height, uint32_t format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, uint32_t sampleCount = 1);
	void createVulkanImage(VulkanImage& image, VkImage vkImage, VkImageView view);
	void destroyVulkanImage(VulkanContext& context, VulkanImage& image);
	void vulkanImageUploadData(VulkanContext& context, VulkanImage& image, void* data, uint32_t width, uint32_t height, uint32_t bytePerPixel, uint32_t layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	struct VulkanSwapchain {

		VkSwapchainKHR swapchain;

		std::vector<VulkanImage> images;

		uint32_t swapchainWidth;
		uint32_t swapchainHeight;
		VkFormat format;
		uint32_t currentSwapchainImageIndex;

	};

	void createVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain, VkSurfaceKHR surface);
	void reCreateVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain, VkSurfaceKHR surface);
	void destroyVulkanSwapchain(VulkanContext& context, VulkanSwapchain& swapchain);

	void vulkanSwapchainAquireNextImage(VulkanContext& context, VulkanSwapchain& swapchain, VkSemaphore signalSemaphore);


	class VulkanRenderpass {

	public:

		VkRenderPass renderpass;

		void create(VulkanContext& context, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies = {});
		void destroy(VulkanContext& context);

	};

	class VulkanFramebuffer {

	public:

		VkFramebuffer framebuffer;

		void create(VulkanContext& context, VulkanRenderpass& renderpass, const std::vector<VulkanImage>& images);
		void destroy(VulkanContext& context);

	};

	class VulkanShaderPack {

	public:

		VkShaderModule vertexShader;
		VkShaderModule fragmentShader;
		std::vector<VkDescriptorSetLayout> layouts;

		void create(VulkanContext& context, const std::filesystem::path& vertexFilePath, const std::filesystem::path& fragmentFilePath);
		void destroy(VulkanContext& context);

	private:

		void reflectShader(VulkanContext& context, const std::vector<uint8_t>& vertexData, const std::vector<uint8_t>& fragmentData);
		VkDescriptorSetLayout createLayout(VulkanContext& context, std::pair<VkShaderStageFlags, SpvReflectDescriptorBinding*>* bindings, uint32_t bindingCount);

	};

	struct VulkanPipelineCreateInfo {

		VulkanShaderPack shaders;

		VulkanRenderpass renderpass;
		uint32_t subpassIndex;

		std::vector<VkFormat> vertexLayout;

		bool depthTestEnabled = false;
		uint8_t sampleCount = 1;

	};

	class VulkanPipeline {

	public:

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VulkanShaderPack shaders;

		void create(VulkanContext& context, VulkanPipelineCreateInfo& createInfo);
		void destroy(VulkanContext& context);

	};

	struct VulkanWindow {

		Window* window;
		VulkanSwapchain swapchain;
		VkSurfaceKHR surface;

	};

}

