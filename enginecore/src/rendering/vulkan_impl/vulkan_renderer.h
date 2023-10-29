#pragma once
#include <glm/glm.hpp>

#include "core/core.h"
#include "vulkan_core.h"

struct GLFWwindow;

namespace ec {

	struct QuadRendererCreateInfo {

		VulkanWindow* window;

	};

	struct QuadVertex {
		glm::vec3 position;
		glm::vec2 uv;
	};

	struct QuadUniformBuffer {
		glm::mat4 transform;
		glm::vec4 color;
	};

	enum class QuadRendererState {

		IN_FRAME,
		OUT_OF_FRAME

	};

	struct VulkanQuadRendererData {

		const uint32_t MAX_QUAD_COUNT = 1000;

		VkSampler sampler;

		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;
		uint32_t imageIndex;
		VulkanPipeline pipeline;
		VulkanRenderpass renderpass;
		std::vector<VulkanFramebuffer> framebuffers;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;

		VulkanBuffer objectDataBuffer;
		uint32_t quadCount = 0;

		VulkanBuffer globalDataBuffer;
		VkDescriptorSet globalDataDescriptorSet;

		VkDescriptorPool descriptorPool;

		QuadRendererState state = QuadRendererState::OUT_OF_FRAME;

	};

	class VulkanQuadRenderer {

	public:

		VulkanQuadRenderer() = default;
		~VulkanQuadRenderer() = default;

		VulkanQuadRenderer(const VulkanQuadRenderer&) = delete;
		VulkanQuadRenderer& operator=(const VulkanQuadRenderer&) = delete;

		VulkanQuadRenderer(const VulkanQuadRenderer&&) = delete;
		VulkanQuadRenderer& operator=(const VulkanQuadRenderer&&) = delete;

		void create(VulkanContext& context, QuadRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void beginFrame(VulkanContext& context, VulkanWindow& window);
		void drawQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, VulkanImage& image, const glm::vec2& srcPos = { 0.0f, 0.0f }, const glm::vec2& srcSize = { 0.0f, 0.0f });

		VkCommandBuffer endFrame(VulkanContext& context);

	private:

		VulkanQuadRendererData m_data;

	};

	class VulkanSynchronisationController {

	public:

		void create(VulkanContext& context);
		void destroy(VulkanContext& context);

		void waitAndBeginFrame(VulkanContext& context, VulkanWindow& window);
		void submitFrameAndPresent(VulkanContext& context, VulkanWindow& window, const std::vector<VkCommandBuffer>& data);
		void waitDeviceIdle(VulkanContext& context);

	private:
		
		VkFence m_fence;
		VkSemaphore m_aquireSemaphore;
		VkSemaphore m_submitSemaphore;

	};

}