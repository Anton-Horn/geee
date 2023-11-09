#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "core/core.h"

struct SpvReflectDescriptorBinding;

namespace ec {

	struct VulkanContextData {

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

	class VulkanContext {

	public:

		VulkanContext() = default;
		~VulkanContext() = default;

		VulkanContext(const VulkanContext&) = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;

		VulkanContext(const VulkanContext&&) = delete;
		VulkanContext& operator=(const VulkanContext&&) = delete;

		void createDefaultVulkanContext(const std::string& applicationName, std::vector<const char*>& additionalWindowInstanceExtensions);
		void create(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);
		void destroy();

		const VulkanContextData& getData() const;

	private:

		VulkanContextData m_data;

		void createInstance(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions);
		void createDevice(const std::vector<const char*>& deviceExtensions);
		void registerDebugCallback();

	};

	enum class MemoryType {

		Device_local,
		Auto,
		Host_local

	};

	class VulkanBuffer {

	public:

		VulkanBuffer() = default;
		~VulkanBuffer() = default;

		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

		VulkanBuffer(const VulkanBuffer&&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&&) = delete;

		void create(VulkanContext& context, uint64_t size, uint32_t usage, MemoryType type = MemoryType::Auto);
		void destroy(VulkanContext& context);

		void uploadData(VulkanContext& context, void* data, uint32_t size, uint32_t offset = 0);
		uint64_t getSize(VulkanContext& context);
		const VkBuffer getBuffer() const;

	private:

		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VkBufferUsageFlags m_usage;

	};


	class VulkanImage {

	public:

		VulkanImage() = default;
		~VulkanImage() = default;

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		VulkanImage(const VulkanImage&&) {};
		VulkanImage& operator=(const VulkanImage&&) {};

		void create(VulkanContext& context, uint32_t width, uint32_t height, uint32_t format = VK_FORMAT_R8G8B8A8_UNORM, uint32_t usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, uint32_t sampleCount = 1);
		void create(VkImage image, VkImageView view, uint32_t width, uint32_t height);
		void destroy(VulkanContext& context);
		void uploadData(VulkanContext& context, void* data, uint32_t width, uint32_t height, uint32_t bytePerPixel, uint32_t layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		const VkImage getImage() const;
		const VkImageView getImageView() const;
		const uint32_t getWidth() const;
		const uint32_t getHeight() const;

	private:

		VkImage m_image;
		VkImageView m_imageView;
		VmaAllocation m_allocation;
		uint32_t m_imageWidth, m_imageHeight;
		bool m_mutable = false;

	};

	class VulkanSwapchain {

	public:

		VulkanSwapchain() = default;
		~VulkanSwapchain() = default;

		VulkanSwapchain(const VulkanSwapchain&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		VulkanSwapchain(const VulkanSwapchain&&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&&) = delete;

		void create(VulkanContext& VulkanContext, VkSurfaceKHR surface);
		void reCreate(VulkanContext& VulkanContext, VkSurfaceKHR surface);
		void destroy(VulkanContext& VulkanContext);

		void aquireNextImage(VulkanContext& VulkanContext, VkSemaphore signalSemaphore);

		uint32_t getWidth() const;
		uint32_t getHeight() const;
		uint32_t getCurrentIndex() const;

		VkFormat getFormat() const;
		const std::vector<VulkanImage>& getImages() const;
		const VkSwapchainKHR getSwapchain() const;

	private:

		VkSwapchainKHR m_swapchain;

		std::vector<VulkanImage> m_images;

		uint32_t m_swapchainWidth;
		uint32_t m_swapchainHeight;
		VkFormat m_format;
		uint32_t m_currentSwapchainImageIndex;

	};

	class VulkanRenderpass {

	public:

		VulkanRenderpass() = default;
		~VulkanRenderpass() = default;

		VulkanRenderpass(const VulkanRenderpass&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&) = delete;

		VulkanRenderpass(const VulkanRenderpass&&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&&) = delete;

		void create(VulkanContext& context, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies = {});
		void destroy(VulkanContext& context);

		const VkRenderPass getRenderpass() const;

	private:

		VkRenderPass m_renderpass;

	};

	class VulkanFramebuffer {

	public:

		VulkanFramebuffer() = default;
		~VulkanFramebuffer() = default;

		VulkanFramebuffer(const VulkanFramebuffer&) = delete;
		VulkanFramebuffer& operator=(const VulkanFramebuffer&) = delete;

		VulkanFramebuffer(const VulkanFramebuffer&&) noexcept {};
		VulkanFramebuffer& operator=(const VulkanFramebuffer&&) noexcept {};

		void create(VulkanContext& context, VulkanRenderpass& renderpass, const std::vector<const VulkanImage*>& images);
		void destroy(VulkanContext& context);

		const VkFramebuffer getFramebuffer() const;

	private:

		VkFramebuffer m_framebuffer;

	};

	class VulkanShaderPack {

	public:

		VulkanShaderPack() = default;
		~VulkanShaderPack() = default;

		VulkanShaderPack(const VulkanShaderPack&) = delete;
		VulkanShaderPack& operator=(const VulkanShaderPack&) = delete;

		VulkanShaderPack(const VulkanShaderPack&&) = delete;
		VulkanShaderPack& operator=(const VulkanShaderPack&&) = delete;

		void create(VulkanContext& context, const std::filesystem::path& vertexFilePath, const std::filesystem::path& fragmentFilePath);
		void destroy(VulkanContext& context);

		const VkShaderModule getVertexShader() const;
		const VkShaderModule getFragementShader() const;
		const std::vector<VkDescriptorSetLayout>& getLayouts() const;

	private:

		VkShaderModule m_vertexShader;
		VkShaderModule m_fragmentShader;
		std::vector<VkDescriptorSetLayout> m_layouts;

		void reflectShader(VulkanContext& context, const std::vector<uint8_t>& vertexData, const std::vector<uint8_t>& fragmentData);
		VkDescriptorSetLayout createLayout(VulkanContext& context, std::pair<VkShaderStageFlags, SpvReflectDescriptorBinding*>* bindings, uint32_t bindingCount);

	};

	struct VulkanPipelineCreateInfo {

		std::filesystem::path vertexShaderFilePath;
		std::filesystem::path fragmentShaderFilePath;

		VulkanRenderpass* renderpass;
		uint32_t subpassIndex;

		std::vector<VkFormat> vertexLayout;

		bool depthTestEnabled = false;
		uint8_t sampleCount = 1;

	};

	class VulkanPipeline {

	public:

		VulkanPipeline() = default;
		~VulkanPipeline() = default;

		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;

		VulkanPipeline(const VulkanPipeline&&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&&) = delete;


		void create(VulkanContext& context, VulkanPipelineCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		const VkPipelineLayout getLayout() const;
		const VulkanShaderPack& getShaders() const;
		const VkPipeline getPipeline() const;

	private:

		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		VulkanShaderPack m_shaders;

	};

	class Window;

	struct VulkanWindow {

		Window* window;
		VkSurfaceKHR surface;
		VulkanSwapchain swapchain;

	};

	struct VulkanModelCreateInfo {

		std::filesystem::path filepath;

	};

	class VulkanModel {

	public:

		void create(VulkanModelCreateInfo& createInfo);
		void destroy();

	};


}

