#include "renderer.h"

#include "vulkan_impl/vulkan_renderer.h"
#include "vulkan_impl/vulkan_utils.h"
#include "stb_image.h"

namespace ec {

	struct RendererData {

		VulkanContext context;
		VulkanWindow window;
		VulkanSynchronisationController synController;
		VulkanQuadRenderer renderer;

		VulkanImage image;

	};

	void Renderer::init(const Window& window) {
	
		m_data = std::make_unique<RendererData>();

		m_data->context.createDefaultVulkanContext("Sandbox", getInstanceExtensions());
		m_data->window.surface = getSurface(m_data->context, window);
		m_data->window.swapchain.create(m_data->context, m_data->window.surface);
		m_data->window.window = (Window*) &window;

		m_data->synController.create(m_data->context);

		QuadRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;

		m_data->renderer.create(m_data->context, createInfo);

		int x, y, channels;
		uint8_t* data = stbi_load("image.png", &x, &y, &channels, 4);

		m_data->image.create(m_data->context, x, y, VK_FORMAT_R8G8B8A8_UNORM);
		m_data->image.uploadData(m_data->context, data, x,y, 4);

		stbi_image_free(data);

	}
	void Renderer::draw() {

		m_data->synController.waitAndBeginFrame(m_data->context, m_data->window);

		m_data->renderer.beginFrame(m_data->context, m_data->window);

		m_data->renderer.drawQuad(m_data->context, {0.0f, 0.0f, 0.0f}, {300.0f, 300.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}, m_data->image);

		m_data->synController.submitFrameAndPresent(m_data->context, m_data->window, { m_data->renderer.endFrame(m_data->context) });

	}
	void Renderer::destroy() {

		m_data->synController.waitDeviceIdle(m_data->context);
		m_data->context.destroy();
	}

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

}
