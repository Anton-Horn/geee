#pragma once
#include <glm/glm.hpp>

#include "core/core.h"
#include "vulkan_core.h"

struct GLFWwindow;

namespace ec {

	struct QuadRendererCreateInfo {

		VulkanWindow window;

	};

	typedef VkCommandBuffer RendererSubmitData;

	class QuadRenderer {

	public:

		void create(VulkanContext& context, QuadRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void beginFrame(VulkanContext& context, Window& window);
		void drawQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, Image& image, const glm::vec2& srcPos = { 0.0f, 0.0f }, const glm::vec2& srcSize = { 0.0f, 0.0f });

		RendererSubmitData endFrame(VulkanContext& context);

		const uint32_t MAX_QUAD_COUNT = 1000;

	private:

		enum class RendererState {

			IN_FRAME,
			OUT_OF_FRAME

		};

		VkSampler m_sampler;

		VkCommandBuffer m_commandBuffer;
		VkCommandPool m_commandPool;
		uint32_t m_imageIndex;
		VulkanPipeline m_pipeline;
		VulkanRenderpass m_renderpass;
		std::vector<VulkanFramebuffer> m_framebuffers;
		VulkanBuffer m_vertexBuffer;
		VulkanBuffer m_indexBuffer;

		VulkanBuffer m_objectDataBuffer;
		uint32_t m_quadCount = 0;

		VulkanBuffer m_globalDataBuffer;
		VkDescriptorSet m_globalDataDescriptorSet;

		VkDescriptorPool m_descriptorPool;

		RendererState m_state = RendererState::OUT_OF_FRAME;

		struct QuadVertex {
			glm::vec3 position;
			glm::vec2 uv;
		};

		struct QuadUniformBuffer {
			glm::mat4 transform;
			glm::vec4 color;
		};


	};


	class SynchronisationController {

	public:

		void create(VulkanContext& context);
		void destroy(VulkanContext& context);

		void waitAndBeginFrame(VulkanContext& context, Window& window);
		void submitFrameAndPresent(VulkanContext& context, Window& window, const std::vector<RendererSubmitData>& data);
		void waitDeviceIdle(VulkanContext& context);

	private:

		VkFence m_fence;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

	};

}