#include "renderer.h"

#include "vulkan_impl/vulkan_renderer.h"
#include "vulkan_impl/vulkan_utils.h"
#include "stb_image.h"


namespace ec {

	struct RendererData {

		VulkanContext context;
		VulkanWindow window;
		VulkanSynchronisationController synController;

		VulkanQuadRenderer quadRenderer;
		VulkanBezierRenderer bezierRenderer;

	};

	void Renderer::init(RendererCreateInfo& rendererCreateInfo) {
	
		m_data = std::make_unique<RendererData>();

		m_quadRendererDrawCallback = std::move(rendererCreateInfo.quadRendererDrawCallback);
		m_bezierRendererDrawCallback = std::move(rendererCreateInfo.bezierRendererDrawCallback);

		m_data->context.createDefaultVulkanContext("Sandbox", getInstanceExtensions());
		m_data->window.surface = getSurface(m_data->context, *rendererCreateInfo.window);
		m_data->window.swapchain.create(m_data->context, m_data->window.surface);
		m_data->window.window = (Window*)rendererCreateInfo.window;

		m_data->synController.create(m_data->context);

		VulkanQuadRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		m_data->quadRenderer.create(m_data->context, createInfo);

		VulkanBezierRendererCreateInfo bezierRendererCreateInfo;
		bezierRendererCreateInfo.window = &m_data->window;
		m_data->bezierRenderer.create(m_data->context, bezierRendererCreateInfo);


	}

	void Renderer::draw() {

		m_data->synController.waitAndBeginFrame(m_data->context, m_data->window);

		m_data->quadRenderer.beginFrame(m_data->context, m_data->window);

		m_quadRendererDrawCallback();

		VkCommandBuffer quadRendererCommandBuffer = m_data->quadRenderer.endFrame(m_data->context);

		m_data->bezierRenderer.beginFrame(m_data->context, m_data->window);

		m_bezierRendererDrawCallback();

		VkCommandBuffer bezierRendererCommandBuffer = m_data->bezierRenderer.endFrame(m_data->context);
		m_data->synController.submitFrameAndPresent(m_data->context, m_data->window, { bezierRendererCommandBuffer, quadRendererCommandBuffer });

	}
	void Renderer::destroy() {

		m_data->synController.waitDeviceIdle(m_data->context);
		
		m_data->quadRenderer.destroy(m_data->context);
		m_data->context.destroy();
		
	}

	void Renderer::drawBezierCurve(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2, const vec4& color)
	{
		m_data->bezierRenderer.drawCurve(m_data->context, *(glm::vec3*)&p1, *(glm::vec3*)&p2, *(glm::vec3*)&c1, *(glm::vec3*)&c2, *(glm::vec4*)&color);
	}


	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	const RendererData& Renderer::getData() const
	{
		return *m_data;
	}

}
