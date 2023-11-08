#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan_renderer.h"
#include "vulkan_utils.h"

namespace ec {

	void VulkanQuadRenderer::create( VulkanContext& context, VulkanQuadRendererCreateInfo& createInfo) {

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

		VkAttachmentDescription colorAttachment = createAttachment(1, createInfo.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
		std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
		std::vector<VkAttachmentReference> resolveAttachments = {};
		std::vector<VkAttachmentReference> inputAttachments = {};
		VkSubpassDescription subpassDescription = createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

		m_data.renderpass.create(context, { colorAttachment }, { subpassDescription });

		VulkanPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.subpassIndex = 0;
		pipelineCreateInfo.renderpass = &m_data.renderpass;
		pipelineCreateInfo.depthTestEnabled = false;
		pipelineCreateInfo.sampleCount = 1;
		pipelineCreateInfo.vertexShaderFilePath = "shaders/VertexShader.spv";
		pipelineCreateInfo.fragmentShaderFilePath = "shaders/FragmentShader.spv";

		pipelineCreateInfo.vertexLayout = { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT };

		m_data.pipeline.create(context, pipelineCreateInfo);

		m_data.framebuffers.resize(createInfo.window->swapchain.getImages().size());

		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].create(context, m_data.renderpass, { &createInfo.window->swapchain.getImages()[i]});
		}

		m_data.commandPool = createCommandPool(context);
		m_data.commandBuffer = allocateCommandBuffer(context, m_data.commandPool);

		// Index-Daten für das Rechteck
		uint32_t indexData[] = {
			0, 1, 2,
			3, 0, 2,
		};

		// Erstellen des Vertex-Buffers
		m_data.vertexBuffer.create(context, sizeof(QuadVertex) * m_data.MAX_QUAD_COUNT * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Host_local);

		// Erstellen des Index-Buffers
		m_data.indexBuffer.create(context, sizeof(indexData), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryType::Auto);
		m_data.indexBuffer.uploadData(context, indexData, sizeof(indexData), 0);

		m_data.objectDataBuffer.create(context, alignToPow2(context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(QuadUniformBuffer)) * m_data.MAX_QUAD_COUNT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryType::Host_local);

		m_data.descriptorPool = createDesciptorPool(context, m_data.MAX_QUAD_COUNT, { {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_data.MAX_QUAD_COUNT}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_data.MAX_QUAD_COUNT } });

		m_data.globalDataBuffer.create(context, alignToPow2(context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(glm::mat4)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryType::Host_local);
		glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
		m_data.globalDataBuffer.uploadData(context, &proj, sizeof(glm::mat4), 0);

		m_data.globalDataDescriptorSet = allocateDescriptorSet(context, context.getData().generalDescriptorPool, m_data.pipeline.getShaders().getLayouts()[0]);
		writeDescriptorUniformBuffer(context, m_data.globalDataDescriptorSet, 0, m_data.globalDataBuffer);

	}
	void VulkanQuadRenderer::destroy(VulkanContext& context) {
		
		for (uint32_t i = 0; i < m_data.framebuffers.size(); i++) {
			m_data.framebuffers[i].destroy(context);
		}
		m_data.renderpass.destroy(context);
		m_data.pipeline.destroy(context);
		vkDestroyCommandPool(context.getData().device, m_data.commandPool, nullptr);
		m_data.vertexBuffer.destroy(context);
		m_data.indexBuffer.destroy(context);

	}

	void VulkanQuadRenderer::beginFrame(VulkanContext& context, VulkanWindow& window)
	{

		EC_ASSERT(m_data.state == QuadRendererState::OUT_OF_FRAME);

		VKA(vkResetDescriptorPool(context.getData().device, m_data.descriptorPool, 0));

		VKA(vkResetCommandPool(context.getData().device, m_data.commandPool, 0));

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKA(vkBeginCommandBuffer(m_data.commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderpassBeginInfo.renderPass = m_data.renderpass.getRenderpass();
		renderpassBeginInfo.framebuffer = m_data.framebuffers[window.swapchain.getCurrentIndex()].getFramebuffer();
		renderpassBeginInfo.renderArea = { 0,0, window.swapchain.getWidth(), window.swapchain.getHeight()};
		renderpassBeginInfo.clearValueCount = 1;
		VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
		renderpassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_data.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getPipeline());

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 0, 1, &m_data.globalDataDescriptorSet, 0, nullptr);

		VkViewport viewport = { 0.0f, 0.0f, (float)window.swapchain.getWidth(), (float)window.swapchain.getHeight(), 0.0f, 1.0f};
		VkRect2D scissor = { {0,0}, {window.swapchain.getWidth(), window.swapchain.getHeight()}};

		vkCmdSetViewport(m_data.commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_data.commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };

		const VkBuffer vertexBuffer = m_data.vertexBuffer.getBuffer();

		vkCmdBindVertexBuffers(m_data.commandBuffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_data.commandBuffer, m_data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		m_data.state = QuadRendererState::IN_FRAME;

	}

	void VulkanQuadRenderer::drawQuad(VulkanContext& context, const glm::vec3& position, const glm::vec3& scale, float angle, const glm::vec4& color, VulkanImage& image, const glm::vec2& srcPos, const glm::vec2& srcSize)
	{

		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		EC_ASSERT(m_data.quadCount != m_data.MAX_QUAD_COUNT);
		QuadUniformBuffer uniformBuffer;
		uniformBuffer.transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) * glm::rotate(glm::mat4(1.0f), angle, { 0.0f, 0.0f, 1.0f });
		uniformBuffer.color = color;

		QuadVertex vertexData[4];

		if (srcSize != glm::vec2(0.0f, 0.0f)) {

			glm::vec2 texPos = srcPos / glm::vec2(image.getWidth(), image.getWidth());
			glm::vec2 texSize = srcSize / glm::vec2(image.getHeight(), image.getHeight());

			vertexData[0] = { glm::vec3(1.0f, 0.0f, 0.0f)  , glm::vec2(texPos.x,texPos.y + texSize.y) };
			vertexData[1] = { glm::vec3(1.0f,  1.0f, 0.0f) , glm::vec2(texPos.x + texSize.x, texPos.y + texSize.y) };
			vertexData[2] = { glm::vec3(0.0f, 1.0f, 0.0f)   , glm::vec2(texPos.x + texSize.x, texPos.y) };
			vertexData[3] = { glm::vec3(0.0f, 0.0f, 0.0f)   , glm::vec2(texPos.x, texPos.y) };

		}
		else {

			vertexData[0] = { glm::vec3(1.0f, 0.0f, 0.0f)  , glm::vec2(1.0f, 0.0f) };    // Oben Rechts
			vertexData[1] = { glm::vec3(1.0f,  1.0f, 0.0f) , glm::vec2(1.0f, 1.0f) };    // Unten rechts
			vertexData[2] = { glm::vec3(0.0f, 1.0f, 0.0f)   , glm::vec2(0.0f, 1.0f) };    // Unten links
			vertexData[3] = { glm::vec3(0.0f, 0.0f, 0.0f)   , glm::vec2(0.0f, 0.0f) };    // Oben links
		}

		m_data.vertexBuffer.uploadData(context, vertexData, sizeof(vertexData), m_data.quadCount * sizeof(vertexData));

		VkDescriptorSet descriptorSet = allocateDescriptorSet(context, m_data.descriptorPool, m_data.pipeline.getShaders().getLayouts()[1]);

		uint32_t alignedSize = alignToPow2(context.getData().deviceProperties.limits.minUniformBufferOffsetAlignment, sizeof(QuadUniformBuffer));
		uint32_t offset = m_data.quadCount * alignedSize;

		m_data.objectDataBuffer.uploadData(context, &uniformBuffer, sizeof(QuadUniformBuffer), offset);

		writeDescriptorUniformBuffer(context, descriptorSet, 0, m_data.objectDataBuffer, true, 0, alignedSize);
		writeCombinedImageSampler(context, descriptorSet, 1, image, m_data.sampler);

		vkCmdBindDescriptorSets(m_data.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data.pipeline.getLayout(), 1, 1, &descriptorSet, 1, &offset);
		vkCmdDrawIndexed(m_data.commandBuffer, 6, 1, 0, m_data.quadCount * 4, 0);

		m_data.quadCount++;

	}

	VkCommandBuffer VulkanQuadRenderer::endFrame(VulkanContext& context)
	{
		EC_ASSERT(m_data.state == QuadRendererState::IN_FRAME);
		vkCmdEndRenderPass(m_data.commandBuffer);

		vkEndCommandBuffer(m_data.commandBuffer);
		m_data.state = QuadRendererState::OUT_OF_FRAME;
		m_data.quadCount = 0;
		return m_data.commandBuffer;
	}

	const VulkanQuadRendererData& VulkanQuadRenderer::getData() const
	{
		return m_data;
	}



}