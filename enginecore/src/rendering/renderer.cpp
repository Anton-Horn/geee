#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "stb_image.h"

#include "renderer.h"
#include "renderer_data.h"

#include "vulkan_impl/vulkan_utils.h"

#include "scene/scene.h"
#include "core/application.h"

namespace ec {

	struct Quad {

		glm::mat4 transform;
		glm::vec4 color;

		Quad(const glm::mat4& transform, const glm::vec4& color) : transform(transform), color(color) {}
		Quad() = default;

	};

	struct RendererSceneData {

		Scene scene;

	};

	void Renderer::create(RendererCreateInfo& rendererCreateInfo) {
	
		

		m_data = new RendererData();
		m_sceneData = new RendererSceneData();
		m_vulkanDataInterface = new VulkanDataInterface();

		m_flags = rendererCreateInfo.flags;


		EC_ASSERT(!(m_flags & RENDERER_DONT_AQUIRE_IMAGE && m_flags & RENDERER_PRESENT_TO_SWAPCHAIN));
		EC_WARN_CON(m_flags & RENDERER_DONT_AQUIRE_IMAGE, "Renderer: renderer wont aquire the image, this needs to be handled by client application!");

		m_data->context.createDefaultVulkanContext("Sandbox", getInstanceExtensions());
		EC_LOG("Renderer: created the vulkan context");

		m_data->window.surface = createSurface(m_data->context, *rendererCreateInfo.window);
		m_data->window.swapchain.create(m_data->context, m_data->window.surface);
		EC_LOG("Renderer: created the swapchain");

		m_data->window.window = (Window*)rendererCreateInfo.window;

		m_data->synController.create(m_data->context);
		EC_LOG("Renderer: created the synchronization controller for vulkan rendering");

		m_data->presentWaitSemaphores.resize(1);
		m_data->presentWaitSemaphores[0] = m_data->synController.getSubmitSemaphore();

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

		createVulkanDataInterface();

		VulkanRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		createInfo.commandBuffer = m_vulkanDataInterface->addCommandBuffer(nullptr);
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);
		EC_LOG("Renderer: created the quad renderer");

		uint32_t presentRendererCreateFlags = QUAD_RENDERER_WAIT_FOR_PREVIOUS_RENDERPASSES;
		if (!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)) presentRendererCreateFlags |= QUAD_RENDERER_WAIT_COLOR_ATTACHMENT_OUTPUT;
		else presentRendererCreateFlags |= QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET;

		createInfo.commandBuffer = m_vulkanDataInterface->addCommandBuffer(nullptr);;
		m_data->presentRenderer.create(m_data->context, createInfo, presentRendererCreateFlags);
		EC_LOG("Renderer: create the present renderer");

		VulkanModelCreateInfo modelCreateInfo = { "data/models/BoomBox.glb", VulkanModelSourceFormat::GLTF};
		m_data->model.create(m_data->context, modelCreateInfo);

		uint32_t data = 0xFFFFFFFF;

		m_data->image.create(m_data->context, 1, 1);
		m_data->image.uploadData(m_data->context, &data, 1, 1, 4);
	


	}

	void Renderer::createVulkanDataInterface()
	{

		m_vulkanDataInterface->addCommandBuffer = [&](VkCommandPool commandPool = nullptr) {
			if (!commandPool) commandPool = m_data->commandPool;
			VkCommandBuffer commandBuffer = allocateCommandBuffer(m_data->context, commandPool);
			m_data->commandBuffers.push_back(commandBuffer);
			return commandBuffer;
			};

		if (!(m_flags & RENDERER_ENABLE_VULKAN_INTERFACE)) return;

		if (!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)) {
			m_vulkanDataInterface->getPresentImageView = [&]() { return m_data->presentRenderer.getData().renderTarget.getImageView(); };
			m_vulkanDataInterface->getPresentImageHandle = [&]() { return m_data->presentRenderer.getData().renderTarget.getImage(); };
		}
		else {
			m_vulkanDataInterface->getPresentImageView = [&]() { EC_ERROR("RENDERER_PRESENT_TO_SWAPCHAIN error"); return nullptr; };
			m_vulkanDataInterface->getPresentImageHandle = [&]() { EC_ERROR("RENDERER_PRESENT_TO_SWAPCHAIN error"); return nullptr; };
		}

		if (m_flags & RENDERER_CUSTOM_QUEUE_SUBMIT) {
			m_vulkanDataInterface->addPresentWaitSemaphore = [&]() {
				VkSemaphore r = createSemaphore(m_data->context);
				m_data->presentWaitSemaphores.push_back(r);
				return r;
				};

			m_vulkanDataInterface->submitFrameSynchronized = [&](const std::vector<VkCommandBuffer>& commandBuffers, bool waitForSwapchain, VkFence signalFence, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore) {
				m_data->synController.submitFrame(m_data->context, commandBuffers, waitForSwapchain, signalFence, waitSemaphore, signalSemaphore);
				};

			m_vulkanDataInterface->addBeginFrameWaitingFence = [&]() { return m_data->synController.addFence(m_data->context); };

		}
		else {
			m_vulkanDataInterface->addPresentWaitSemaphore = [&]() {
					EC_ERROR("RENDERER_CUSTOM_QUEUE_SUBMIT error");
					return nullptr;
				};

			m_vulkanDataInterface->submitFrameSynchronized = [&](const std::vector<VkCommandBuffer>& commandBuffers, bool waitForSwapchain, VkFence signalFence, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore) {
					EC_ERROR("RENDERER_CUSTOM_QUEUE_SUBMIT error");
					return nullptr;
				};

			m_vulkanDataInterface->addBeginFrameWaitingFence = [&]() { 
					EC_ERROR("RENDERER_CUSTOM_QUEUE_SUBMIT error");
					return nullptr; 
				};
		}

		if (m_flags & RENDERER_DONT_AQUIRE_IMAGE) {
			m_vulkanDataInterface->aquireNextSwapchainImage = [&](VkSemaphore signalSemaphore, bool& recreateSwapchain) {
				m_data->window.swapchain.aquireNextImage(m_data->context, signalSemaphore, recreateSwapchain);
				};
		}
		else {
			m_vulkanDataInterface->aquireNextSwapchainImage = [&](VkSemaphore signalSemaphore, bool& recreateSwapchain) {
					EC_ERROR("RENDERER_DONT_AQUIRE_IMAGE error");
					return nullptr;
				};
		}

		m_vulkanDataInterface->context = &m_data->context;
		m_vulkanDataInterface->window = &m_data->window;

	}

	void Renderer::beginFrame(RendererBeginFrameInfo& beginInfo)
	{

		if (!(m_flags & RENDERER_DONT_AQUIRE_IMAGE))
			if (m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)
				m_data->synController.waitAndAquireImage(m_data->context, m_data->window, m_recreateSwapchain);
			else
				m_data->synController.waitAndAquireImage(m_data->context, m_data->window, m_recreateSwapchain, false);
		else
			m_data->synController.wait(m_data->context);

		if (beginInfo.recreateSwapchain) *beginInfo.recreateSwapchain = m_recreateSwapchain;

		VKA(vkResetCommandPool(m_data->context.getData().device, m_data->commandPool, 0));

	}

	void Renderer::drawFrame()
	{

		if (m_recreateSwapchain) return;

		VKA(vkResetDescriptorPool(m_data->context.getData().device, m_data->rpfDescriptorPool, 0));

		//Quad renderer

		m_data->quadRenderer.beginFrame(m_data->context);

		auto view = m_sceneData->scene.raw().view<TransformComponent, QuadRenderComponent>();

		for (auto& entity : view) {

			auto& [transform, quad] = view.get(entity);

			m_data->quadRenderer.drawTexturedQuad(m_data->context, transform.transform, quad.color, m_data->image);
		}

		m_data->quadRenderer.endFrame(m_data->context);

		// Presenting, rendering to swapchain

		m_data->presentRenderer.beginFrame(m_data->context);
		m_data->presentRenderer.drawTexturedQuad(m_data->context, { 0.0, 0.0, 0.0f }, { m_data->window.swapchain.getWidth(), m_data->window.swapchain.getHeight(), 1.0f }, 0.0f, glm::vec4{ 1.0f }, m_data->quadRenderer.getData().renderTarget);
		m_data->presentRenderer.endFrame(m_data->context);

	}
	void Renderer::submitFrame()
	{
		if (m_recreateSwapchain) return;

		m_data->synController.submitFrame(m_data->context, m_data->commandBuffers, m_flags & RENDERER_PRESENT_TO_SWAPCHAIN ? true : false);

	}
	void Renderer::presentFrame(RendererPresentFrameInfo& presentInfo)
	{
		if (m_recreateSwapchain) return;

		m_data->window.swapchain.present(m_data->context, m_data->presentWaitSemaphores, m_recreateSwapchain);

		
		if (presentInfo.recreateSwapchain) *presentInfo.recreateSwapchain = m_recreateSwapchain;
	
	}
	void Renderer::destroy() {

		m_data->synController.waitDeviceIdle(m_data->context);
		m_data->image.destroy(m_data->context);
		m_data->model.destroy(m_data->context);
	
		//skip first one because it will get deleted by the synchronization controller
		for (uint32_t i = 1; i < m_data->presentWaitSemaphores.size(); i++) {
			vkDestroySemaphore(m_data->context.getData().device, m_data->presentWaitSemaphores[i], nullptr);
		}

		m_data->synController.destroy(m_data->context);

		m_data->window.swapchain.destroy(m_data->context);

		vkDestroyDescriptorPool(m_data->context.getData().device, m_data->rpfDescriptorPool, nullptr);
		vkDestroyCommandPool(m_data->context.getData().device, m_data->commandPool, nullptr);

		m_data->quadRenderer.destroy(m_data->context);
		m_data->presentRenderer.destroy(m_data->context);

		destroySurface(m_data->context, m_data->window.surface);
		m_data->context.destroy();
		
		delete m_vulkanDataInterface;
		delete m_data;
		delete m_sceneData;

	}

	void Renderer::waitDeviceIdle()
	{
		m_data->synController.waitDeviceIdle(m_data->context);
	}


	void Renderer::recreate()
	{

		m_data->synController.waitDeviceIdle(m_data->context);

		m_data->commandBuffers.clear();	

		m_data->window.swapchain.recreate(m_data->context, m_data->window.surface);
		m_data->quadRenderer.destroy(m_data->context);
		m_data->presentRenderer.destroy(m_data->context);

		VulkanRendererCreateInfo createInfo;
		createInfo.window = &m_data->window;
		createInfo.commandBuffer = m_vulkanDataInterface->addCommandBuffer(nullptr);
		createInfo.rpfDescriptorPool = m_data->rpfDescriptorPool;

		m_data->quadRenderer.create(m_data->context, createInfo);

		uint32_t presentRendererCreateFlags = QUAD_RENDERER_WAIT_FOR_PREVIOUS_RENDERPASSES;
		if (!(m_flags & RENDERER_PRESENT_TO_SWAPCHAIN)) presentRendererCreateFlags |= QUAD_RENDERER_WAIT_COLOR_ATTACHMENT_OUTPUT;
		else presentRendererCreateFlags |= QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET;

		createInfo.commandBuffer = m_vulkanDataInterface->addCommandBuffer(nullptr);
		m_data->presentRenderer.create(m_data->context, createInfo, presentRendererCreateFlags);

		m_recreateSwapchain = false;
	}

	

	bool Renderer::handleEvents(const Event& event)
	{
		if (event.eventType == EventType::ApplicationRecreateEvent) {
			recreate();
			return false;
		}
		return true;
	}

	const VulkanDataInterface& Renderer::getVulkanData() const
	{
		EC_ASSERT(m_flags & RENDERER_ENABLE_VULKAN_INTERFACE);
		return *m_vulkanDataInterface;
	}

}
