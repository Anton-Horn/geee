#include <spirv_reflect.h>

#include "vulkan_core.h"
#include "utils/file_utils.h"

namespace ec {


	void VulkanShaderModule::create(VulkanContext& context, const std::filesystem::path& filePath)
	{

		std::vector<uint8_t> vertexData = ecUtilsReadBinaryFile(filePath);

		SpvReflectShaderModule reflectedModule;
		SpvReflectResult result = spvReflectCreateShaderModule(vertexData.size(), vertexData.data(), &reflectedModule);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t vertexLayoutCount = 0;
		result = spvReflectEnumerateDescriptorSets(&reflectedModule, &vertexLayoutCount, nullptr);
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> vertexReflectedLayouts;
		vertexReflectedLayouts.resize(vertexLayoutCount);

		result = spvReflectEnumerateDescriptorSets(&reflectedModule, &vertexLayoutCount, vertexReflectedLayouts.data());
		EC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = vertexData.size();
		createInfo.pCode = (uint32_t*)vertexData.data();

		VKA(vkCreateShaderModule(context.getData().device, &createInfo, nullptr, &m_module));

	}

	void VulkanShaderModule::destroy(VulkanContext& context)
	{

		vkDestroyShaderModule(context.getData().device, m_module, nullptr);

	}

}