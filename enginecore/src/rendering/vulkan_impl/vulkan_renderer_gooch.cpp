#include <glm/gtc/matrix_transform.hpp>
#include "vulkan_renderer.h"
#include "vulkan_utils.h"

namespace ec {

	void VulkanGoochRenderer::create(VulkanContext& context, VulkanRendererCreateInfo& createInfo) {

		m_window = createInfo.window;

		VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias = 0;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;

		VKA(vkCreateSampler(context.getData().device, &samplerCreateInfo, nullptr, &m_data.sampler));

		VkAttachmentDescription depthAttachment = createDepthAttachment(1);

		VkAttachmentDescription colorAttachment = createAttachment(1, createInfo.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
		std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
		std::vector<VkAttachmentReference> resolveAttachments = {};
		std::vector<VkAttachmentReference> inputAttachments = {};
		VkAttachmentReference depthAttachmentReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments, depthAttachmentReference);

		m_data.renderpass.create(context, { colorAttachment, depthAttachment }, { subpassDescription });

		VulkanPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.subpassIndex = 0;
		pipelineCreateInfo.renderpass = &m_data.renderpass;
		pipelineCreateInfo.depthTestEnabled = true;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.vertexShaderFilePath = "shaders/gooch_vertex.spv";
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/gooch_fragment.spv";

		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.pipeline.create(context, pipelineCreateInfo);

		m_data.depthImages.resize(createInfo.window->swapchain.getImages().size());

		for (uint32_t i = 0; i < m_data.depthImages.size(); i++) {
			m_data.depthImages[i].create(context, m_window->swapchain.getWidth(), m_window->swapchain.getHeight(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1);
		}

		m_data.framebuffers.resize(createInfo.window->swapchain.getImages().size());

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].create(context, m_data.renderpass, { &createInfo.window->swapchain.getImages()[i], &m_data.depthImages[i] });
		}

		m_data.commandBuffer = allocateCommandBuffer(context, createInfo.commandPool);

		m_data.objectUniformBuffer.create(context, MemoryType::Host_local, m_data.MAX_MESHES_COUNT);

		m_data.descriptorPool = createInfo.rpfDescriptorPool;


		m_data.camera.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.01f, -100.0f);
		m_data.camera.viewTransform = glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 1.0f });

		m_data.globalUniformBuffer.create(context,  MemoryType::Host_local);
		
		GlobalGoochRendererUniformBuffer uniformBuffer;

		uniformBuffer.view = m_data.camera.viewTransform;
		uniformBuffer.proj = m_data.camera.projection;
		uniformBuffer.cameraPosition = { 0.0f, 0.0f, 0.0f, 0.0f };
		uniformBuffer.lightPosition = { 5.0f, 10.0f, -2.0f, 0.0f };

		m_data.globalUniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(GlobalGoochRendererUniformBuffer), 0);

		m_data.globalDataDescriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.pipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.globalDataDescriptorSet, 0, m_data.globalUniformBuffer.buffer);

	}

	void VulkanGoochRenderer::beginFrame(VulkanContext& context)
	{

		EC_ASSERT(m_data.state == GoochRendererState::OUT_OF_FRAME);

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKA(vkBeginCommandBuffer(m_data.commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderpassBeginInfo.renderPass = m_data.renderpass.getRenderpass();
		renderpassBeginInfo.framebuffer = m_data.framebuffers[m_window->swapchain.getCurrentIndex()].getFramebuffer();
		renderpassBeginInfo.renderArea = { 0,0, m_window->swapchain.getWidth(), m_window->swapchain.getHeight() };
		renderpassBeginInfo.clearValueCount = 2;
		VkClearValue clearValues[2] = { {0.1f, 0.1f, 0.102f, 1.0f}, {1.0f, 0} };
		renderpassBeginInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_data.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getPipeline());

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 0, 1, &m_data.globalDataDescriptorSet, 0, nullptr);

		VkViewport viewport = { 0.0f, 0.0f, (float)m_window->swapchain.getWidth(), (float)m_window->swapchain.getHeight(), 0.0f, 1.0f };
		VkRect2D scissor = { {0,0}, {m_window->swapchain.getWidth(), m_window->swapchain.getHeight()} };

		vkCmdSetViewport(m_data.commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_data.commandBuffer, 0, 1, &scissor);

		m_data.state = GoochRendererState::IN_FRAME;

	}

	VkCommandBuffer VulkanGoochRenderer::endFrame(VulkanContext& context)
	{
		EC_ASSERT(m_data.state == GoochRendererState::IN_FRAME);
		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);
		m_data.state = GoochRendererState::OUT_OF_FRAME;
		m_data.modelCount = 0;
		return m_data.commandBuffer;
	}

	void VulkanGoochRenderer::drawModel(VulkanContext& context, const VulkanModel& model, const glm::mat4& modelTransform)
	{

		EC_ASSERT(m_data.state == GoochRendererState::IN_FRAME);
		EC_ASSERT(m_data.modelCount != m_data.MAX_MESHES_COUNT);

		GoochRendererUniformBuffer uniformBuffer;
		uniformBuffer.modelTransform = modelTransform;

		VkDescriptorSet descriptorSet = allocateDescriptorSet(context, m_data.descriptorPool, m_data.pipeline.getShaders().getLayouts()[1]);

		uint32_t alignedSize = alignToPow2((uint32_t)context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(GoochRendererUniformBuffer));
		uint32_t offset = m_data.modelCount * alignedSize;

		m_data.objectUniformBuffer.buffer.uploadData(context, &uniformBuffer, sizeof(QuadUniformBuffer), offset);

		writeDescriptorUniformBuffer(context, descriptorSet, 0, m_data.objectUniformBuffer.buffer, true, 0, alignedSize);
		writeCombinedImageSampler(context, descriptorSet, 1, model.getMat().albedo, m_data.sampler);

		VkBuffer vb = model.getMesh().vertexBuffer.getBuffer();

		VkDeviceSize vbOffset = { 0 };

		vkCmdBindVertexBuffers(m_data.commandBuffer, 0, 1, &vb, &vbOffset);
		vkCmdBindIndexBuffer(m_data.commandBuffer, model.getMesh().indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 1, 1, &descriptorSet, 1, &offset);
		vkCmdDrawIndexed(m_data.commandBuffer, model.getMesh().indexCount, 1, 0, 0, 0);

		m_data.modelCount++;

	}

	void VulkanGoochRenderer::destroy(VulkanContext& context) {

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].destroy(context);
		}
		m_data.renderpass.destroy(context);
		m_data.pipeline.destroy(context);
		
		m_data.globalUniformBuffer.destroy(context);
		m_data.objectUniformBuffer.destroy(context);

		for (uint32_t i = 0; i < m_data.depthImages.size(); i++) {
			m_data.depthImages[i].destroy(context);
		}
		vkDestroySampler(context.getData().device, m_data.sampler, nullptr);
	}

}