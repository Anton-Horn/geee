#include <glm/gtc/matrix_transform.hpp>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"


namespace ec {

	void VulkanBezierRenderer::create(const VulkanContext& context, VulkanRendererCreateInfo& createInfo)
	{

		m_window = createInfo.window;
		VkAttachmentDescription colorAttachment = createAttachment(1, createInfo.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
		std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
		std::vector<VkAttachmentReference> resolveAttachments = {};
		std::vector<VkAttachmentReference> inputAttachments = {};
		VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

		m_data.renderpass.create(context, { colorAttachment }, { subpassDescription });

		VulkanPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.depthTestEnabled = false;
		pipelineCreateInfo.renderpass = &m_data.renderpass;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.subpassIndex = 0;
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/bezier_fragment.spv";
		pipelineCreateInfo.vertexShaderFilePath = "shaders/bezier_vertex.spv";
		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.pipeline.create(context, pipelineCreateInfo);

		m_data.framebuffers.resize(createInfo.window->swapchain.getImages().size());
		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {

			m_data.framebuffers[i].create(context, m_data.renderpass, { &createInfo.window->swapchain.getImages()[i] });

		}

		m_data.commandBuffer = createInfo.commandBuffer;

		// Index-Daten für das Rechteck
		uint32_t indexData[] = {
			0, 1, 2,
			2, 3, 1,
		};

		// Erstellen des Vertex-Buffers
		m_data.vertexBuffer.create(context, sizeof(QuadVertex) * m_data.MAX_CURVE_COUNT * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Auto);

		// Erstellen des Index-Buffers
		m_data.indexBuffer.create(context, sizeof(indexData), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Auto);
		m_data.indexBuffer.uploadData(context, indexData, sizeof(indexData), 0);

		m_data.descriptorPool = createInfo.rpfDescriptorPool;

		m_data.objectUniformBuffer.create(context, MemoryType::Host_local, m_data.MAX_CURVE_COUNT);
		m_data.globalUniformBuffer.create(context, MemoryType::Device_local);
		
		GlobalBezierUniformBuffer uniformBuffer;
		uniformBuffer.screenSize = { createInfo.window->swapchain.getWidth(), createInfo.window->swapchain.getHeight()};
		uniformBuffer.viewProj = glm::ortho(createInfo.window->swapchain.getWidth() / -2.0f, createInfo.window->swapchain.getWidth() / 2.0f, createInfo.window->swapchain.getHeight() / 2.0f, createInfo.window->swapchain.getHeight() / -2.0f, 1000.0f, -1000.0f);
		
		m_data.globalUniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(GlobalBezierUniformBuffer), 0);
		m_data.globalDataDescriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.pipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.globalDataDescriptorSet, 0, m_data.globalUniformBuffer.buffer);

	}

	void VulkanBezierRenderer::destroy(const VulkanContext& context)
	{

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].destroy(context);
		}
		m_data.renderpass.destroy(context);
		m_data.pipeline.destroy(context);

		m_data.vertexBuffer.destroy(context);
		m_data.indexBuffer.destroy(context);

		m_data.globalUniformBuffer.destroy(context);
		m_data.objectUniformBuffer.destroy(context);
		
	}

	void VulkanBezierRenderer::beginFrame(const VulkanContext& context)
	{
		EC_ASSERT(m_data.state == QuadRendererState::OUT_OF_FRAME);

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

		vkCmdBindPipeline(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getPipeline());

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 0, 1, &m_data.globalDataDescriptorSet, 0, nullptr);

		VkViewport viewport = { 0.0f, 0.0f, (float)m_window->swapchain.getWidth(), (float)m_window->swapchain.getHeight(), 0.0f, 1.0f };
		VkRect2D scissor = { {0,0}, {m_window->swapchain.getWidth(), m_window->swapchain.getHeight()} };

		vkCmdSetViewport(m_data.commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_data.commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };

		const VkBuffer vertexBuffer = m_data.vertexBuffer.getBuffer();

		vkCmdBindVertexBuffers(m_data.commandBuffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_data.commandBuffer, m_data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		m_data.state = QuadRendererState::IN_FRAME;

	}

	void VulkanBezierRenderer::drawCurve(const VulkanContext& context,const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2, const glm::vec4& color)
	{

		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		EC_ASSERT(m_data.quadCount != m_data.MAX_CURVE_COUNT);

		BezierUniformBuffer uniformBuffer;

		//{100.0f, 100.0f, 0.0f}, { 200.0f, 200.0f, 0.0f }, { 150.0f, 100.0f, 0.0f }, { 100.0f, 150.0f, 0.0f }

		float x = glm::min(glm::min(c1.x, c2.x), glm::min(p1.x, p2.x));
		float y = glm::max(glm::max(c1.y, c2.y), glm::max(p1.y, p2.y));

		float maxX = glm::max(glm::max(c1.x, c2.x), glm::max(p1.x, p2.x));
		float minY = glm::min(glm::min(c1.y, c2.y), glm::min(p1.y, p2.y));
		//100, 200 / 200 100
		glm::vec3 position = {x - 50.0f, y + 50.0f, 1.0f};
		glm::vec3 size = {maxX, minY, 1.0f};
		
		size = glm::abs(position - size);
		size += glm::vec3(100.0f, 100.0f, 0.0f);

		uniformBuffer.color = color;

		uniformBuffer.c1 = glm::vec4{ c1, 1.0f };
		uniformBuffer.c2 = glm::vec4{ c2, 1.0f };
		uniformBuffer.p1 = glm::vec4{ p1, 1.0f };
		uniformBuffer.p2 = glm::vec4{ p2, 1.0f };

		uniformBuffer.transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size) * glm::rotate(glm::mat4(1.0f), 0.0f, { 0.0f, 0.0f, 1.0f });

		QuadVertex vertexData[4];

		vertexData[0] = { glm::vec3(0.0f, 0.0f, 0.0f)  , glm::vec2(0.0f, 0.0f)};    // Oben links
		vertexData[1] = { glm::vec3(1.0f,  0.0f, 0.0f) , glm::vec2(1.0f, 0.0f)};    // Oben rechts
		vertexData[2] = { glm::vec3(0.0f, -1.0f, 0.0f)   , glm::vec2(0.0f, 1.0f)};    // Unten links
		vertexData[3] = { glm::vec3(1.0f, -1.0f, 0.0f)   , glm::vec2(1.0f, 1.0f)};    // Unten rechts

		m_data.vertexBuffer.uploadData(context, vertexData, sizeof(vertexData), m_data.quadCount * sizeof(vertexData));

		VkDescriptorSet descriptorSet = allocateDescriptorSet(context, m_data.descriptorPool, m_data.pipeline.getShaders().getLayouts()[1]);

		uint32_t alignedSize = alignToPow2((uint32_t)context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(BezierUniformBuffer));
		uint32_t offset = m_data.quadCount * alignedSize;

		m_data.objectUniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(BezierUniformBuffer), offset);

		writeDescriptorUniformBuffer(context, descriptorSet, 0, m_data.objectUniformBuffer.buffer, true, 0, alignedSize);

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 1, 1, &descriptorSet, 1, &offset);
		vkCmdDrawIndexed(m_data.commandBuffer, 6, 1, 0, m_data.quadCount * 4, 0);

		m_data.quadCount++;

	}

	void VulkanBezierRenderer::endFrame(const VulkanContext& context)
	{
		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);
		m_data.state = QuadRendererState::OUT_OF_FRAME;
		m_data.quadCount = 0;
		
	}


}
