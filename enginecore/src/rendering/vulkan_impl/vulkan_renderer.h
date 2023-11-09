#pragma once
#include <glm/glm.hpp>

#include "core/core.h"
#include "vulkan_core.h"

#define FRAMES_IN_FLIGHT 2

struct GLFWwindow;

namespace ec {

	struct VulkanQuadRendererCreateInfo {

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

		void create(VulkanContext& context, VulkanQuadRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void beginFrame(VulkanContext& context, VulkanWindow& window);
		void drawQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, VulkanImage& image, const glm::vec2& srcPos = { 0.0f, 0.0f }, const glm::vec2& srcSize = { 0.0f, 0.0f });

		VkCommandBuffer endFrame(VulkanContext& context);

		const VulkanQuadRendererData& getData() const;

	private:

		VulkanQuadRendererData m_data;

	};

	struct VulkanBezierRendererCreateInfo {

		VulkanWindow* window;

	};

	struct VulkanBezierRendererData {

		VulkanRenderpass renderpass;
		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;
		VulkanPipeline pipeline;
		std::vector<VulkanFramebuffer> framebuffers;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;
		const uint32_t MAX_CURVE_COUNT = 1000;

		VulkanBuffer objectDataBuffer;
		uint32_t quadCount = 0;

		VulkanBuffer globalDataBuffer;
		VkDescriptorSet globalDataDescriptorSet;

		VkDescriptorPool descriptorPool;
		QuadRendererState state = QuadRendererState::OUT_OF_FRAME;

	};

	struct BezierUniformBuffer {

		glm::vec4 p1;
		glm::vec4 p2;
		glm::vec4 c1;
		glm::vec4 c2;
		glm::mat4 transform;
		glm::vec4 color;
	};

	struct GlobalBezierUniformBuffer {

		glm::mat4 viewProj;
		glm::vec2 screenSize;

	};

	class VulkanBezierRenderer {

	public:

		void create(VulkanContext& context, VulkanBezierRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void drawCurve(VulkanContext& context, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2, const glm::vec4& color);

		void beginFrame(VulkanContext& context, VulkanWindow& window);
		VkCommandBuffer endFrame(VulkanContext& context);

	private:

		VulkanBezierRendererData m_data;

	};

	struct VulkanGoochRendererData {

		VulkanRenderpass renderpass;
		VulkanPipeline pipeline;

		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;

	};

	struct VulkanGoochRendererCreateInfo {

		VulkanWindow* window;

	};

	class VulkanGoochRenderer {

	public:

		void create(VulkanGoochRendererCreateInfo& createInfo);
		void destroy();

	private:

		VulkanGoochRendererData m_data;

	};

}