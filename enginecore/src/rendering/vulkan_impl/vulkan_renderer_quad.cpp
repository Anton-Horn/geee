#include <glm/gtc/matrix_transform.hpp>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"

namespace ec {

	void VulkanQuadRenderer::create(const VulkanContext& context, VulkanRendererCreateInfo& createInfo, uint32_t createFlags) {

		m_window = createInfo.window;
		m_flags = createFlags;

		m_data.sampler = createSampler(context);

		if (!(m_flags & QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET)) {

			VkAttachmentDescription colorAttachment = createAttachment(1, createInfo.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
			std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
			std::vector<VkAttachmentReference> resolveAttachments = {};
			std::vector<VkAttachmentReference> inputAttachments = {};
			VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

			VkSubpassDependency waitBefore = {};
			waitBefore.srcSubpass = VK_SUBPASS_EXTERNAL;
			waitBefore.dstSubpass = 0;
			waitBefore.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			waitBefore.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			waitBefore.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			waitBefore.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubpassDependency waitAfter = {};
			waitAfter.srcSubpass = 0;
			waitAfter.dstSubpass = VK_SUBPASS_EXTERNAL;
			waitAfter.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			waitAfter.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			waitAfter.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			waitAfter.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			std::vector<VkSubpassDependency> dependencies;
			dependencies.reserve(2);

			if (m_flags & QUAD_RENDERER_WAIT_FOR_PREVIOUS_RENDERPASSES)
				dependencies.push_back(waitBefore);
			
			if (m_flags &QUAD_RENDERER_WAIT_COLOR_ATTACHMENT_OUTPUT) 
				dependencies.push_back(waitAfter);

			m_data.renderpass.create(context, { colorAttachment }, { subpassDescription }, dependencies);

			m_data.renderTarget.create(context, createInfo.window->swapchain.getWidth(), createInfo.window->swapchain.getHeight(), createInfo.window->swapchain.getFormat(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);

			m_data.framebuffer.create(context, m_data.renderpass, { &m_data.renderTarget });

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

		pipelineCreateInfo.renderpass = (m_flags & (uint32_t) QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET) ? (VulkanRenderpass*) &createInfo.window->swapchain.getRenderpass() : &m_data.renderpass;
		pipelineCreateInfo.depthTestEnabled = false;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.vertexShaderFilePath = "shaders/VertexShader.spv";
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/FragmentShader.spv";

		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.texturedQuadPipeline.create(context, pipelineCreateInfo);

		m_data.texturedQuadObjectUniformBuffer.create(context, MemoryType::Host_local, m_data.MAX_QUAD_COUNT);

		m_data.texturedQuadGlobalUniformBuffer.create(context, MemoryType::Device_local);
		glm::mat4 proj = glm::ortho(createInfo.window->swapchain.getWidth() / -2.0f, createInfo.window->swapchain.getWidth() / 2.0f, createInfo.window->swapchain.getHeight() / -2.0f, createInfo.window->swapchain.getHeight() / 2.0f, 1000.0f, -1000.0f);
		m_data.texturedQuadGlobalUniformBuffer.buffer.uploadData(context, &proj, sizeof(glm::mat4), 0);

		m_data.texturedQuadGlobalDataDescriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.texturedQuadPipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.texturedQuadGlobalDataDescriptorSet, 0, m_data.texturedQuadGlobalUniformBuffer.buffer);


	}
	void VulkanQuadRenderer::destroy(const VulkanContext& context) {
		
		if (!(m_flags & QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET)) {
			m_data.framebuffer.destroy(context);
			m_data.renderTarget.destroy(context);
			m_data.renderpass.destroy(context);
		}

		m_data.vertexBuffer.destroy(context);
		m_data.indexBuffer.destroy(context);

		m_data.texturedQuadPipeline.destroy(context);
		vkDestroySampler(context.getData().device, m_data.sampler, nullptr);
	
		m_data.texturedQuadGlobalUniformBuffer.destroy(context);
		m_data.texturedQuadObjectUniformBuffer.destroy(context);

	}

	void VulkanQuadRenderer::beginFrame(const VulkanContext& context)
	{

		EC_ASSERT(m_data.state == QuadRendererState::OUT_OF_FRAME);

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKA(vkBeginCommandBuffer(m_data.commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderpassBeginInfo.renderPass = (m_flags & QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET) ? m_window->swapchain.getRenderpass().getRenderpass() : m_data.renderpass.getRenderpass();
		renderpassBeginInfo.framebuffer = (m_flags & QUAD_RENDERER_SWAPCHAIN_IS_RENDER_TARGET) ? m_window->swapchain.getFramebuffers()[m_window->swapchain.getCurrentIndex()].getFramebuffer() : m_data.framebuffer.getFramebuffer();
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


	void VulkanQuadRenderer::drawTexturedQuad(const VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color,const VulkanImage& image)
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

	void VulkanQuadRenderer::drawTexturedQuad(const VulkanContext& context, const glm::mat4& transform, const glm::vec4& color, const VulkanImage& image)
	{

		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		EC_ASSERT(m_data.texturedQuadCount != m_data.MAX_QUAD_COUNT);

		QuadUniformBuffer uniformBuffer;
		uniformBuffer.transform = transform;
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

	void VulkanQuadRenderer::endFrame(const VulkanContext& context)
	{
		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);
		m_data.state = QuadRendererState::OUT_OF_FRAME;
		m_data.texturedQuadCount = 0;
		
	}

	const VulkanQuadRendererData& VulkanQuadRenderer::getData() const
	{
		return m_data;
	}



}