#pragma once
#include <glm/glm.hpp>

#include "core/core.h"
#include "vulkan_core.h"

#define FRAMES_IN_FLIGHT 2

struct GLFWwindow;

namespace ec {

	struct VulkanRendererCreateInfo {

		VulkanWindow* window;
		VkCommandPool commandPool;
		VkDescriptorPool rpfDescriptorPool; // reset per frame descriptor Pool

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

		VkCommandBuffer commandBuffer;

		VulkanRenderpass renderpass;
		std::vector<VulkanFramebuffer> framebuffers;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;

		QuadRendererState state = QuadRendererState::OUT_OF_FRAME;

		VkDescriptorPool descriptorPool;

		//textured quad
		VkSampler sampler;

		VulkanPipeline texturedQuadPipeline;
		UniformBuffer<QuadUniformBuffer> texturedQuadObjectUniformBuffer;

		UniformBuffer<glm::mat4> texturedQuadGlobalUniformBuffer;
		VkDescriptorSet texturedQuadGlobalDataDescriptorSet;

		uint32_t texturedQuadCount = 0;

	};

	class VulkanQuadRenderer {

	public:

		VulkanQuadRenderer() = default;
		~VulkanQuadRenderer() = default;

		VulkanQuadRenderer(const VulkanQuadRenderer&) = delete;
		VulkanQuadRenderer& operator=(const VulkanQuadRenderer&) = delete;

		VulkanQuadRenderer(const VulkanQuadRenderer&&) = delete;
		VulkanQuadRenderer& operator=(const VulkanQuadRenderer&&) = delete;

		void create(VulkanContext& context, VulkanRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void beginFrame(VulkanContext& context);

		void drawTexturedQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, VulkanImage& image);
	
		VkCommandBuffer endFrame(VulkanContext& context);

		const VulkanQuadRendererData& getData() const;

	private:

		VulkanQuadRendererData m_data;
		VulkanWindow* m_window;

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

	struct VulkanBezierRendererData {

		VulkanRenderpass renderpass;
		VkCommandBuffer commandBuffer;

		VulkanPipeline pipeline;
		std::vector<VulkanFramebuffer> framebuffers;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;
		const uint32_t MAX_CURVE_COUNT = 1000;

		UniformBuffer<BezierUniformBuffer> objectUniformBuffer;
		uint32_t quadCount = 0;

		UniformBuffer<GlobalBezierUniformBuffer> globalUniformBuffer;
		VkDescriptorSet globalDataDescriptorSet;

		VkDescriptorPool descriptorPool;
		QuadRendererState state = QuadRendererState::OUT_OF_FRAME;

	};

	class VulkanBezierRenderer {

	public:

		VulkanBezierRenderer() = default;
		~VulkanBezierRenderer() = default;

		VulkanBezierRenderer(const VulkanBezierRenderer&) = delete;
		VulkanBezierRenderer& operator=(const VulkanBezierRenderer&) = delete;

		VulkanBezierRenderer(const VulkanBezierRenderer&&) = delete;
		VulkanBezierRenderer& operator=(const VulkanBezierRenderer&&) = delete;

		void create(VulkanContext& context, VulkanRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void drawCurve(VulkanContext& context, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2, const glm::vec4& color);

		void beginFrame(VulkanContext& context);
		VkCommandBuffer endFrame(VulkanContext& context);

	private:

		VulkanWindow* m_window;
		VulkanBezierRendererData m_data;

	};

	enum class GoochRendererState {

		IN_FRAME,
		OUT_OF_FRAME

	};

	struct GoochRendererUniformBuffer {

		glm::mat4 modelTransform;

	};

	struct Camera {

		glm::mat4 viewTransform;
		glm::mat4 projection;

	};

	struct GlobalGoochRendererUniformBuffer {

		glm::mat4 proj;
		glm::mat4 view;
		glm::vec4 cameraPosition;
		glm::vec4 lightPosition;

	};

	struct VulkanGoochRendererData {

		VulkanRenderpass renderpass;
		VulkanPipeline pipeline;

		VkCommandBuffer commandBuffer;

		VkSampler sampler;
		std::vector<VulkanFramebuffer> framebuffers;
		std::vector<VulkanImage> depthImages;

		UniformBuffer<QuadUniformBuffer> objectUniformBuffer;
		uint32_t modelCount = 0;

		UniformBuffer<GlobalGoochRendererUniformBuffer> globalUniformBuffer;
		VkDescriptorSet globalDataDescriptorSet;

		VkDescriptorPool descriptorPool;
		GoochRendererState state = GoochRendererState::OUT_OF_FRAME;

		const uint32_t MAX_MESHES_COUNT = 1000;

		Camera camera;
		glm::vec3 lightPosition;

	};

	class VulkanGoochRenderer {

	public:

		VulkanGoochRenderer() = default;
		~VulkanGoochRenderer() = default;

		VulkanGoochRenderer(const VulkanGoochRenderer&) = delete;
		VulkanGoochRenderer& operator=(const VulkanGoochRenderer&) = delete;
		
		VulkanGoochRenderer(const VulkanGoochRenderer&&) = delete;
		VulkanGoochRenderer& operator=(const VulkanGoochRenderer&&) = delete;

		void create(VulkanContext& context, VulkanRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		void beginFrame(VulkanContext& context);
		VkCommandBuffer endFrame(VulkanContext& context);

		void drawModel(VulkanContext& context,const VulkanModel& model,const glm::mat4& modelTransform);

	private:

		VulkanWindow* m_window;
		VulkanGoochRendererData m_data;

	};

	struct MandelbrotQuadUniformBuffer {

		glm::mat4 viewProj;
		glm::mat4 transform;
		glm::vec4 spec;

	};

	struct VulkanMandelbrotRendererData {

		VkCommandBuffer commandBuffer;

		VulkanRenderpass renderpass;
		std::vector<VulkanFramebuffer> framebuffers;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;

		VulkanPipeline pipeline;

		UniformBuffer<MandelbrotQuadUniformBuffer> uniformBuffer;
		VkDescriptorSet descriptorSet;

	};

	class VulkanMandelbrotRenderer {

	public:

		VulkanMandelbrotRenderer() = default;
		~VulkanMandelbrotRenderer() = default;

		VulkanMandelbrotRenderer(const VulkanMandelbrotRenderer&) = delete;
		VulkanMandelbrotRenderer& operator=(const VulkanMandelbrotRenderer&) = delete;

		VulkanMandelbrotRenderer(const VulkanMandelbrotRenderer&&) = delete;
		VulkanMandelbrotRenderer& operator=(const VulkanMandelbrotRenderer&&) = delete;

		void create(VulkanContext& context, VulkanRendererCreateInfo& createInfo);
		void destroy(VulkanContext& context);

		VkCommandBuffer drawMandelbrot(VulkanContext& context,const glm::mat4& transform, const glm::vec2& cstart, float zoom, float iterations);

	private:

		VulkanWindow* m_window;
		VulkanMandelbrotRendererData m_data;

	};

}