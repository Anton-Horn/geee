#include <spirv_reflect.h>

#include "vulkan_core.h"
#include "vulkan_utils.h"

#include "utils/file_utils.h"

namespace ec {

	void VulkanShaderPack::create(VulkanContext& context, const std::filesystem::path& vertexFilePath, const std::filesystem::path& fragmentFilePath) {


		std::vector<uint8_t> vertexData = ecUtilsReadBinaryFile(vertexFilePath);
		std::vector<uint8_t> fragmentData = ecUtilsReadBinaryFile(fragmentFilePath);

		reflectShader(context, vertexData, fragmentData);

		{
			VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			createInfo.codeSize = vertexData.size();
			createInfo.pCode = (uint32_t*)vertexData.data();

			VKA(vkCreateShaderModule(context.getData().device, &createInfo, nullptr, &m_vertexShader));
		}

		{
			VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			createInfo.codeSize = fragmentData.size();
			createInfo.pCode = (uint32_t*)fragmentData.data();

			VKA(vkCreateShaderModule(context.getData().device, &createInfo, nullptr, &m_fragmentShader));
		}

	}

	void VulkanShaderPack::destroy(VulkanContext& context) {

		vkDestroyShaderModule(context.getData().device, m_vertexShader, nullptr);
		vkDestroyShaderModule(context.getData().device, m_fragmentShader, nullptr);
		for (VkDescriptorSetLayout layout : m_layouts) {
			vkDestroyDescriptorSetLayout(context.getData().device, layout, nullptr);
		}
	}

	const VkShaderModule VulkanShaderPack::getVertexShader() const
	{
		return m_vertexShader;
	}

	const VkShaderModule VulkanShaderPack::getFragementShader() const
	{
		return m_fragmentShader;
	}

	const std::vector<VkDescriptorSetLayout>& VulkanShaderPack::getLayouts() const
	{
		return m_layouts;
	}

	void VulkanShaderPack::reflectShader(VulkanContext& context, const std::vector<uint8_t>& vertexData, const std::vector<uint8_t>& fragmentData)
	{

		//Vertex Shader
		SpvReflectShaderModule vertexModule;
		SpvReflectResult result = spvReflectCreateShaderModule(vertexData.size(), vertexData.data(), &vertexModule);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t vertexLayoutCount = 0;
		result = spvReflectEnumerateDescriptorSets(&vertexModule, &vertexLayoutCount, nullptr);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> vertexReflectedLayouts;
		vertexReflectedLayouts.resize(vertexLayoutCount);

		result = spvReflectEnumerateDescriptorSets(&vertexModule, &vertexLayoutCount, vertexReflectedLayouts.data());
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		//Fragment Shader

		SpvReflectShaderModule fragmentModule;
		result = spvReflectCreateShaderModule(fragmentData.size(), fragmentData.data(), &fragmentModule);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t fragmentLayoutCount = 0;
		result = spvReflectEnumerateDescriptorSets(&fragmentModule, &fragmentLayoutCount, nullptr);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> fragmentReflectedLayouts;
		fragmentReflectedLayouts.resize(fragmentLayoutCount);

		result = spvReflectEnumerateDescriptorSets(&fragmentModule, &fragmentLayoutCount, fragmentReflectedLayouts.data());
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t highestSetIndex = 0;

		for (auto set : fragmentReflectedLayouts) {
			highestSetIndex = std::max(set->set, highestSetIndex);
		}

		for (auto set : vertexReflectedLayouts) {
			highestSetIndex = std::max(set->set, highestSetIndex);
		}

		m_layouts.resize(highestSetIndex + 1);

		for (uint32_t i = 0; i < highestSetIndex + 1; i++) {

			std::vector<std::pair<VkShaderStageFlags, SpvReflectDescriptorBinding*>> bindings;

			for (auto set : fragmentReflectedLayouts) {
				if (set->set == i) {
					for (uint32_t j = 0; j < set->binding_count; j++) {

						bindings.push_back({ VK_SHADER_STAGE_FRAGMENT_BIT, set->bindings[j] });
					}
				}
			}

			for (auto set : vertexReflectedLayouts) {
				if (set->set == i) {
					for (uint32_t j = 0; j < set->binding_count; j++) {
						bindings.push_back({ VK_SHADER_STAGE_VERTEX_BIT, set->bindings[j] });
					}
				}
			}

			if (!bindings.empty()) {
				m_layouts[i] = createLayout(context, bindings.data(), (uint32_t)bindings.size());
			}

		}

		spvReflectDestroyShaderModule(&vertexModule);

	}

	VkDescriptorSetLayout VulkanShaderPack::createLayout(VulkanContext& context, std::pair<VkShaderStageFlags, SpvReflectDescriptorBinding*>* bindings, uint32_t bindingCount)
	{

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.resize(bindingCount);

		for (uint32_t i = 0; i < bindingCount; i++) {
			SpvReflectDescriptorBinding* binding = bindings[i].second;

			std::string bindingName = binding->name;
			SpvReflectDescriptorType bindingType = binding->descriptor_type;
			if (bindingName.substr(0, 9) == "_dynamic_" && binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				bindingType = SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			}

			layoutBindings[i] = { binding->binding, (VkDescriptorType)bindingType, binding->count, bindings[i].first , nullptr };
		}
		return createSetLayout(context, layoutBindings);
	}
}