#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"

#include "vulkan_impl/vulkan_renderer.h"
#include "vulkan_impl/vulkan_synchronisation.h"
#include "vulkan_impl/vulkan_utils.h"
#include "stb_image.h"


namespace ec {

	struct RendererData {

		VulkanContext context;
		VulkanWindow window;
		VulkanSynchronisationController synController;

		VulkanQuadRenderer quadRenderer;
		VulkanBezierRenderer bezierRenderer;
		VulkanGoochRenderer goochRenderer;
		VulkanMandelbrotRenderer mandelbrotRenderer;

		VulkanModel model;
		VulkanImage image;

		VkCommandBuffer additionalCommandBuffer;
		VkCommandPool commandPool;
		VkDescriptorPool rpfDescriptorPool;

		MandelbrotSpec mandelbrotSpec = { 0.0f, 0.0f, 30.0f, 1.0f };


	};

	void Renderer::init(RendererCreateInfo& rendererCreateInfo) {
	
		m_data = std::make_unique<RendererData>();

		m_callbacks = std::move(rendererCreateInfo.callbacks);

		m_data->context.createDefaultVulkanContext("Sandbox", getInstanceExtensions());
		m_data->window.surface = createSurface(m_data->context, *rendererCreateInfo.window);
		m_data->window.swapchain.create(m_data->context, m_data->window.surface);
		m_data->window.window = (Window*)rendererCreateInfo.window;

		m_data->synController.create(m_data->context);

		m_data->commandPool = createCommandPool(m_data->context);

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		m_data->rpfDescriptorPool = createDesciptorPool(m_data->context, 1000, poolSizes);

		VulkanRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		createInfo.commandPool = m_data->commandPool;
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);
		m_data->bezierRenderer.create(m_data->context, createInfo);
		m_data->goochRenderer.create(m_data->context, createInfo);
		m_data->mandelbrotRenderer.create(m_data->context, createInfo);

		VulkanModelCreateInfo modelCreateInfo = { "data/models/BoomBox.glb", VulkanModelSourceFormat::GLTF};
		m_data->model.create(m_data->context, modelCreateInfo);


		uint32_t data = 0xFFFFFFFF;

		m_data->image.create(m_data->context, 1, 1);
		m_data->image.uploadData(m_data->context, &data, 1, 1, 4);

	}

	float rotation = 0.0f;

	void Renderer::draw() {

		bool recreateSwapchain = false;

		m_data->synController.waitAndBeginFrame(m_data->context, m_data->window, recreateSwapchain);

		if (recreateSwapchain) recreate();

		VKA(vkResetDescriptorPool(m_data->context.getData().device, m_data->rpfDescriptorPool, 0));

		VKA(vkResetCommandPool(m_data->context.getData().device, m_data->commandPool, 0));

		//Quad renderer

		m_data->quadRenderer.beginFrame(m_data->context);

		m_data->quadRenderer.drawTexturedQuad(m_data->context, glm::vec3{100.0f, 100.0f, 10.0f}, {200.0f , 200.0f , 0.0f }, 0.0f, glm::vec4(1.0f), m_data->image);

		if (m_callbacks.quadRendererDrawCallback.has_value())
		m_callbacks.quadRendererDrawCallback.value()();

		VkCommandBuffer quadRendererCommandBuffer = m_data->quadRenderer.endFrame(m_data->context);

		//Bezier renderer

		m_data->bezierRenderer.beginFrame(m_data->context);

		if (m_callbacks.bezierRendererDrawCallback.has_value())
		m_callbacks.bezierRendererDrawCallback.value()();

		VkCommandBuffer bezierRendererCommandBuffer = m_data->bezierRenderer.endFrame(m_data->context);

		//Gooch renderer

		m_data->goochRenderer.beginFrame(m_data->context);

		rotation += 0.01f;

		m_data->goochRenderer.drawModel(m_data->context, m_data->model, glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, 5.0f}) * glm::scale(glm::mat4(1.0f), {100.0f, 100.0f, 100.0f}) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 1.0f, 0.0f}));

		if (m_callbacks.goochRendererDrawCallback.has_value())
		m_callbacks.goochRendererDrawCallback.value()();

		VkCommandBuffer goochRendererCommandBuffer = m_data->goochRenderer.endFrame(m_data->context);

		if (m_callbacks.drawCallback.has_value())
		m_callbacks.drawCallback.value()();

		//glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(m_data->window.swapchain.getWidth(), m_data->window.swapchain.getHeight(), 1.0f));
		//VkCommandBuffer mandelbrotRendererCommandBuffer = m_data->mandelbrotRenderer.drawMandelbrot(m_data->context, transform, glm::vec2(m_data->mandelbrotSpec.csX, m_data->mandelbrotSpec.csY), 1.0f /  m_data->mandelbrotSpec.zoom, m_data->mandelbrotSpec.iterations);

		m_data->synController.submitFrameAndPresent(m_data->context, m_data->window, { quadRendererCommandBuffer, bezierRendererCommandBuffer, goochRendererCommandBuffer, m_data->additionalCommandBuffer}, recreateSwapchain);

		if (recreateSwapchain) recreate();

	}
	void Renderer::destroy() {

		m_data->synController.waitDeviceIdle(m_data->context);
		m_data->image.destroy(m_data->context);
		m_data->model.destroy(m_data->context);
	
		m_data->synController.destroy(m_data->context);

		m_data->window.swapchain.destroy(m_data->context);

		vkDestroyDescriptorPool(m_data->context.getData().device, m_data->rpfDescriptorPool, nullptr);
		vkDestroyCommandPool(m_data->context.getData().device, m_data->commandPool, nullptr);

		m_data->quadRenderer.destroy(m_data->context);
		m_data->goochRenderer.destroy(m_data->context);
		m_data->bezierRenderer.destroy(m_data->context);
		m_data->mandelbrotRenderer.destroy(m_data->context);

		destroySurface(m_data->context, m_data->window.surface);
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

	RendererData& Renderer::getData()
	{
		return *m_data;
	}

	void Renderer::recreate()
	{
		m_data->synController.waitDeviceIdle(m_data->context);

		m_data->window.swapchain.recreate(m_data->context, m_data->window.surface);
		m_data->quadRenderer.destroy(m_data->context);
		m_data->goochRenderer.destroy(m_data->context);
		m_data->bezierRenderer.destroy(m_data->context);
		m_data->mandelbrotRenderer.destroy(m_data->context);

		VulkanRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		createInfo.commandPool = m_data->commandPool;
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);
		m_data->bezierRenderer.create(m_data->context, createInfo);
		m_data->goochRenderer.create(m_data->context, createInfo);
		m_data->mandelbrotRenderer.create(m_data->context, createInfo);

		if (m_callbacks.recreateCallback.has_value())
		m_callbacks.recreateCallback.value()();

	}

}
