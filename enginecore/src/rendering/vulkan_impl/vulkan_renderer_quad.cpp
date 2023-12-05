#include <glm/gtc/matrix_transform.hpp>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"

namespace ec {

	void VulkanQuadRenderer::create( VulkanContext& context, VulkanRendererCreateInfo& createInfo) {

		m_window = createInfo.window;

		m_data.sampler = createSampler(context);

		VkAttachmentDescription colorAttachment = createAttachment(1, createInfo.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
		std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
		std::vector<VkAttachmentReference> resolveAttachments = {};
		std::vector<VkAttachmentReference> inputAttachments = {};
		VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

		m_data.renderpass.create(context, { colorAttachment }, { subpassDescription });

		m_data.framebuffers.resize(createInfo.window->swapchain.getImages().size());

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].create(context, m_data.renderpass, { &createInfo.window->swapchain.getImages()[i]});
		}

		m_data.commandBuffer = allocateCommandBuffer(context, createInfo.commandPool);

		// Index-Daten für das Rechteck
		uint32_t indexData[] = {
			0, 1, 2,
			3, 0, 2,
		};

		// Erstellen des Vertex-Buffers
		m_data.vertexBuffer.create(context, sizeof(QuadVertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Device_local);

		QuadVertex vertexData[4];

		vertexData[0] = { {0.5f, 0.5f, 0.0f}, {1.0f, 1.0f} };    // Oben Rechts
		vertexData[1] = { {0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} };   // Unten Rechts
		vertexData[2] = { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} };  // Unten Links
		vertexData[3] = { {-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f} };   // Oben Links

		m_data.vertexBuffer.uploadData(context, vertexData, sizeof(vertexData));

		// Erstellen des Index-Buffers
		m_data.indexBuffer.create(context, sizeof(indexData), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Device_local);
		m_data.indexBuffer.uploadData(context, indexData, sizeof(indexData), 0);
		
		m_data.descriptorPool = createInfo.rpfDescriptorPool;

		//textured Quad Pipeline

		VulkanPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.subpassIndex = 0;
		pipelineCreateInfo.renderpass = &m_data.renderpass;
		pipelineCreateInfo.depthTestEnabled = false;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.vertexShaderFilePath = "shaders/VertexShader.spv";
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/FragmentShader.spv";

		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.texturedQuadPipeline.create(context, pipelineCreateInfo);

		m_data.texturedQuadObjectUniformBuffer.create(context, MemoryType::Host_local, m_data.MAX_QUAD_COUNT);

		m_data.texturedQuadGlobalUniformBuffer.create(context, MemoryType::Device_local);
		glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1000.0f, 0.0f);
		m_data.texturedQuadGlobalUniformBuffer.buffer.uploadData(context, &proj, sizeof(glm::mat4), 0);

		m_data.texturedQuadGlobalDataDescriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.texturedQuadPipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.texturedQuadGlobalDataDescriptorSet, 0, m_data.texturedQuadGlobalUniformBuffer.buffer);


	}
	void VulkanQuadRenderer::destroy(VulkanContext& context) {
		
		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].destroy(context);
		}
		
		m_data.renderpass.destroy(context);
		m_data.vertexBuffer.destroy(context);
		m_data.indexBuffer.destroy(context);

		m_data.texturedQuadPipeline.destroy(context);
		vkDestroySampler(context.getData().device, m_data.sampler, nullptr);
	
		m_data.texturedQuadGlobalUniformBuffer.destroy(context);
		m_data.texturedQuadObjectUniformBuffer.destroy(context);

	}

	void VulkanQuadRenderer::beginFrame(VulkanContext& context)
	{

		EC_ASSERT(m_data.state == QuadRendererState::OUT_OF_FRAME);

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKA(vkBeginCommandBuffer(m_data.commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderpassBeginInfo.renderPass = m_data.renderpass.getRenderpass();
		renderpassBeginInfo.framebuffer = m_data.framebuffers[m_window->swapchain.getCurrentIndex()].getFramebuffer();
		renderpassBeginInfo.renderArea = { 0,0, m_window->swapchain.getWidth(), m_window->swapchain.getHeight()};
		renderpassBeginInfo.clearValueCount = 1;
		VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
		renderpassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_data.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.texturedQuadPipeline.getPipeline());

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.texturedQuadPipeline.getLayout(), 0, 1, &m_data.texturedQuadGlobalDataDescriptorSet, 0, nullptr);

		VkViewport viewport = { 0.0f, 0.0f, (float)m_window->swapchain.getWidth(), (float)m_window->swapchain.getHeight(), 0.0f, 1.0f};
		VkRect2D scissor = { {0,0}, {m_window->swapchain.getWidth(), m_window->swapchain.getHeight()}};

		vkCmdSetViewport(m_data.commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_data.commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };

		const VkBuffer vertexBuffer = m_data.vertexBuffer.getBuffer();

		vkCmdBindVertexBuffers(m_data.commandBuffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_data.commandBuffer, m_data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		m_data.state = QuadRendererState::IN_FRAME;

	}


	void VulkanQuadRenderer::drawTexturedQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, VulkanImage& image)
	{

		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		EC_ASSERT(m_data.texturedQuadCount != m_data.MAX_QUAD_COUNT);

		QuadUniformBuffer uniformBuffer;
		uniformBuffer.transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) * glm::rotate(glm::mat4(1.0f), angle, { 0.0f, 0.0f, 1.0f });
		uniformBuffer.color = color;

		VkDescriptorSet descriptorSet = allocateDescriptorSet(context, m_data.descriptorPool, m_data.texturedQuadPipeline.getShaders().getLayouts()[1]);

		uint32_t alignedSize = alignToPow2((uint32_t)context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(QuadUniformBuffer));
		uint32_t offset = m_data.texturedQuadCount * alignedSize;

		m_data.texturedQuadObjectUniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(QuadUniformBuffer), offset);

		writeDescriptorUniformBuffer(context, descriptorSet, 0, m_data.texturedQuadObjectUniformBuffer.buffer, true, 0, alignedSize);
		writeCombinedImageSampler(context, descriptorSet, 1, image, m_data.sampler);

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.texturedQuadPipeline.getLayout(), 1, 1, &descriptorSet, 1, &offset);
		vkCmdDrawIndexed(m_data.commandBuffer, 6, 1, 0, 0, 0);

		m_data.texturedQuadCount++;

	}

	VkCommandBuffer VulkanQuadRenderer::endFrame(VulkanContext& context)
	{
		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);
		m_data.state = QuadRendererState::OUT_OF_FRAME;
		m_data.texturedQuadCount = 0;
		return m_data.commandBuffer;
	}

	const VulkanQuadRendererData& VulkanQuadRenderer::getData() const
	{
		return m_data;
	}



}