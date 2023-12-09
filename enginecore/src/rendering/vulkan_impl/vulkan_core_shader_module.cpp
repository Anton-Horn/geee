#include <spirv_cross/spirv_cross.hpp>

#include "vulkan_core.h"
#include "utils/file_utils.h"



namespace ec {


	void VulkanShaderModule::create(VulkanContext& context, const std::filesystem::path& filePath, VkShaderStageFlags shaderStage)
	{

		std::vector<uint8_t> vertexData = ecUtilsReadBinaryFile(filePath);

		spirv_cross::Compiler compiler((uint32_t*)vertexData.data(), vertexData.size() / 4);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (spirv_cross::Resource r : resources.uniform_buffers) {

			uint32_t set = compiler.get_decoration(r.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(r.id, spv::DecorationBinding);

			VulkanShaderResource resource;
			resource.set = set;
			resource.binding = binding;
			resource.shaderStage = shaderStage;

			if (r.name.find("dynamic") != std::string::npos) {
				resource.type = VulkanShaderResourceType::DYNAMIC_UNIFORM_BUFFER;
			}
			else {
				resource.type = VulkanShaderResourceType::UNIFROM_BUFFER;
			}

			m_resources.push_back(resource);
			m_setCount = std::max(set, m_setCount);
		}

		for (spirv_cross::Resource r : resources.sampled_images) {

			uint32_t binding = compiler.get_decoration(r.id, spv::DecorationBinding);
			uint32_t set = compiler.get_decoration(r.id, spv::DecorationDescriptorSet);

			VulkanShaderResource resource;
			resource.set = set;
			resource.binding = binding;
			resource.type = VulkanShaderResourceType::IMAGE_SAMPLER;
			resource.shaderStage = shaderStage;

			m_resources.push_back(resource);

			m_setCount = std::max(set, m_setCount);
		}		

		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = vertexData.size();
		createInfo.pCode = (uint32_t*)vertexData.data();

		VKA(vkCreateShaderModule(context.getData().device, &createInfo, nullptr, &m_module));

	}

	void VulkanShaderModule::destroy(VulkanContext& context)
	{

		vkDestroyShaderModule(context.getData().device, m_module, nullptr);

	}

	const VkShaderModule VulkanShaderModule::getModule() const
	{
		return m_module;
	}

	const std::vector<VulkanShaderResource>& VulkanShaderModule::getResources() const
	{
		return m_resources;
	}

	uint32_t VulkanShaderModule::getDescriptorSetCount()
	{
		return m_setCount;
	}

}