#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "stb_image.h"

#include "renderer.h"
#include "renderer_data.h"

#include "vulkan_impl/vulkan_utils.h"

#include "scene/scene.h"

namespace ec {

	struct Quad {

		glm::mat4 transform;
		glm::vec4 color;

		Quad(const glm::mat4& transform, const glm::vec4& color) : transform(transform), color(color) {}
		Quad() = default;

	};

	struct RendererSceneData {

		std::vector<Quad> quads;

	};

	void Renderer::create(RendererCreateInfo& rendererCreateInfo) {
	
		m_data = std::make_unique<RendererData>();
		m_sceneData = std::make_unique<RendererSceneData>();
		m_flags = rendererCreateInfo.flags;

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
		createInfo.commandBuffer = (VkCommandBuffer) getCommandBuffer();
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);

		uint32_t presentRendererCreateFlags = QUAD_RENDERER_WAIT_FOR_PREVIOUS_RENDERPASSES;
		if (!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)) presentRendererCreateFlags |= QUAD_RENDERER_WAIT_COLOR_ATTACHMENT_OUTPUT;
		else presentRendererCreateFlags |= QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET;

		createInfo.commandBuffer = (VkCommandBuffer)getCommandBuffer();
		m_data->presentRenderer.create(m_data->context, createInfo, presentRendererCreateFlags);

		VulkanModelCreateInfo modelCreateInfo = { "data/models/BoomBox.glb", VulkanModelSourceFormat::GLTF};
		m_data->model.create(m_data->context, modelCreateInfo);

		uint32_t data = 0xFFFFFFFF;

		m_data->image.create(m_data->context, 1, 1);
		m_data->image.uploadData(m_data->context, &data, 1, 1, 4);
		
		m_sceneData->quads.reserve(m_data->quadRenderer.getData().MAX_QUAD_COUNT);	
	}

	void Renderer::setSceneData(const Scene& scene)
	{

		m_sceneData->quads.clear();

		auto view = scene.raw().view<TransformComponent, QuadRenderComponent>();

		for (auto& entity : view) {

			auto& [transform, quad] = view.get(entity);
			
			m_sceneData->quads.emplace_back(transform.transform, quad.color);
		}

	}

	void Renderer::beginFrame(RendererBeginFrameInfo& beginInfo)
	{

		bool recreateSwapchain = false;
		m_data->synController.waitAndAquireImage(m_data->context, m_data->window, recreateSwapchain);
		if (recreateSwapchain) recreate();

		if (beginInfo.recreateSwapchain) *beginInfo.recreateSwapchain = recreateSwapchain;

		VKA(vkResetCommandPool(m_data->context.getData().device, m_data->commandPool, 0));

	}

	void Renderer::drawFrame()
	{

		VKA(vkResetDescriptorPool(m_data->context.getData().device, m_data->rpfDescriptorPool, 0));

		//Quad renderer

		m_data->quadRenderer.beginFrame(m_data->context);

		for (Quad& quad : m_sceneData->quads) {

			m_data->quadRenderer.drawTexturedQuad(m_data->context, quad.transform, quad.color, m_data->image);

		}

		m_data->quadRenderer.endFrame(m_data->context);

		// Presenting, rendering to swapchain

		m_data->presentRenderer.beginFrame(m_data->context);
		m_data->presentRenderer.drawTexturedQuad(m_data->context, { 0.0, 0.0, 0.0f }, { m_data->window.swapchain.getWidth(), m_data->window.swapchain.getHeight(), 1.0f }, 0.0f, glm::vec4{ 1.0f }, m_data->quadRenderer.getData().renderTarget);
		m_data->presentRenderer.endFrame(m_data->context);

	}
	void Renderer::submitFrame()
	{

		m_data->synController.submitFrame(m_data->context, m_data->window, m_data->commandBuffers);

	}
	void Renderer::presentFrame(RendererPresentFrameInfo& presentInfo)
	{
		bool recreateSwapchain = false;
		m_data->window.swapchain.present(m_data->context, { m_data->synController.getSubmitSemaphore() }, recreateSwapchain);

		if (recreateSwapchain)
		recreate();
		
		if (presentInfo.recreateSwapchain) *presentInfo.recreateSwapchain = recreateSwapchain;
	
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
		m_data->presentRenderer.destroy(m_data->context);

		destroySurface(m_data->context, m_data->window.surface);
		m_data->context.destroy();
		
	}

	void Renderer::waitDeviceIdle()
	{
		m_data->synController.waitDeviceIdle(m_data->context);
	}


	void* Renderer::getPresentImageHandle()
	{
		EC_ASSERT(!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN));
		return m_data->presentRenderer.getData().renderTarget.getImage();
	}

	void* Renderer::getPresentImageView()
	{
		EC_ASSERT(!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN));
		return m_data->presentRenderer.getData().renderTarget.getImageView();
	}

	void* Renderer::getCommandBuffer(void* commandPool)
	{
		if (commandPool) {
			VkCommandBuffer commandBuffer = allocateCommandBuffer(m_data->context, (VkCommandPool) commandPool);
			m_data->commandBuffers.push_back(commandBuffer);
			return commandBuffer;
		}
		else {
			VkCommandBuffer commandBuffer = allocateCommandBuffer(m_data->context, m_data->commandPool);
			m_data->commandBuffers.push_back(commandBuffer);
			return commandBuffer;
		}
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

		m_data->commandBuffers.clear();

		m_data->synController.waitDeviceIdle(m_data->context);

		m_data->window.swapchain.recreate(m_data->context, m_data->window.surface);
		m_data->quadRenderer.destroy(m_data->context);
		m_data->presentRenderer.destroy(m_data->context);

		VulkanRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		createInfo.commandBuffer = (VkCommandBuffer)getCommandBuffer();
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);

		uint32_t presentRendererCreateFlags = QUAD_RENDERER_WAIT_FOR_PREVIOUS_RENDERPASSES;
		if (!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)) presentRendererCreateFlags |= QUAD_RENDERER_WAIT_COLOR_ATTACHMENT_OUTPUT;
		else presentRendererCreateFlags |= QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET;

		createInfo.commandBuffer = (VkCommandBuffer)getCommandBuffer();
		m_data->presentRenderer.create(m_data->context, createInfo, presentRendererCreateFlags);

	}

}
