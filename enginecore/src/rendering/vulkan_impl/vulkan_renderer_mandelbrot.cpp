#include <glm/gtc/matrix_transform.hpp>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"

namespace ec {

	void VulkanMandelbrotRenderer::create(const VulkanContext& context, VulkanRendererCreateInfo& createInfo) {
		
		m_window = createInfo.window;
		VulkanWindow* window = m_window;

		VkAttachmentDescription colorAttachment = createAttachment(1, window->swapchain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
		std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
		std::vector<VkAttachmentReference> resolveAttachments = {};
		std::vector<VkAttachmentReference> inputAttachments = {};
		VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

		m_data.renderpass.create(context, { colorAttachment }, { subpassDescription });

		m_data.framebuffers.resize(window->swapchain.getImages().size());

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].create(context, m_data.renderpass, { &window->swapchain.getImages()[i] });
		}

		m_data.commandBuffer = createInfo.commandBuffer;

		// Index-Daten für das Rechteck
		uint32_t indexData[] = {
			0, 1, 2,
			3, 0, 2,
		};

		// Erstellen des Vertex-Buffers
		m_data.vertexBuffer.create(context, sizeof(QuadVertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Device_local);

		QuadVertex vertexData[4];

		vertexData[0] = { glm::vec3(1.0f, 0.0f, 0.0f)  , glm::vec2(1.0f, 0.0f) };    // Oben Rechts
		vertexData[1] = { glm::vec3(1.0f,  1.0f, 0.0f) , glm::vec2(1.0f, 1.0f) };    // Unten rechts
		vertexData[2] = { glm::vec3(0.0f, 1.0f, 0.0f)   , glm::vec2(0.0f, 1.0f) };    // Unten links
		vertexData[3] = { glm::vec3(0.0f, 0.0f, 0.0f)   , glm::vec2(0.0f, 0.0f) };    // Oben links

		m_data.vertexBuffer.uploadData(context, vertexData, sizeof(vertexData));

		// Erstellen des Index-Buffers
		m_data.indexBuffer.create(context, sizeof(indexData), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Device_local);
		m_data.indexBuffer.uploadData(context, indexData, sizeof(indexData), 0);

		//mandel brot set Pipeline
		VulkanPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.subpassIndex = 0;
		pipelineCreateInfo.renderpass = &m_data.renderpass;
		pipelineCreateInfo.depthTestEnabled = false;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.vertexShaderFilePath = "shaders/mandelbrotset_vertex.spv";
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/mandelbrotset_fragment.spv";

		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.pipeline.create(context, pipelineCreateInfo);

		m_data.uniformBuffer.create(context, MemoryType::Device_local);

		m_data.descriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.pipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.descriptorSet, 0, m_data.uniformBuffer.buffer);

	}
	void VulkanMandelbrotRenderer::destroy(const VulkanContext& context) {

		m_data.pipeline.destroy(context);
		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].destroy(context);
		}
		m_data.renderpass.destroy(context);
		m_data.vertexBuffer.destroy(context);
		m_data.indexBuffer.destroy(context);

		m_data.uniformBuffer.destroy(context);
	}

	void VulkanMandelbrotRenderer::drawMandelbrot(const VulkanContext& context, const glm::mat4& transform, const glm::vec2& cstart, float zoom, float iterations) {


		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKA(vkBeginCommandBuffer(m_data.commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderpassBeginInfo.renderPass = m_data.renderpass.getRenderpass();
		renderpassBeginInfo.framebuffer = m_data.framebuffers[m_window->swapchain.getCurrentIndex()].getFramebuffer();
		renderpassBeginInfo.renderArea = { 0,0, m_window->swapchain.getWidth(), m_window->swapchain.getHeight() };
		renderpassBeginInfo.clearValueCount = 1;
		VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
		renderpassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_data.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline	.getPipeline());

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 0, 1, &m_data.descriptorSet, 0, nullptr);

		VkViewport viewport = { 0.0f, 0.0f, (float)m_window->swapchain.getWidth(), (float)m_window->swapchain.getHeight(), 0.0f, 1.0f };
		VkRect2D scissor = { {0,0}, {m_window->swapchain.getWidth(), m_window->swapchain.getHeight()} };

		vkCmdSetViewport(m_data.commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_data.commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };

		const VkBuffer vertexBuffer = m_data.vertexBuffer.getBuffer();

		vkCmdBindVertexBuffers(m_data.commandBuffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_data.commandBuffer, m_data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		MandelbrotQuadUniformBuffer uniformBuffer;
		uniformBuffer.transform = transform;
		uniformBuffer.viewProj = glm::ortho(0.0f, (float) m_window->swapchain.getWidth(), 0.0f, (float) m_window->swapchain.getHeight());
		uniformBuffer.spec = glm::vec4(cstart, iterations, zoom);

		m_data.uniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(MandelbrotQuadUniformBuffer), 0);

		vkCmdDrawIndexed(m_data.commandBuffer, 6, 1, 0, 0, 0);

		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);

	}

}