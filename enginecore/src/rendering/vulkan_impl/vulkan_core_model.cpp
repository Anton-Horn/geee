#include "vulkan_core.h"

#include "utils/file_utils.h"

namespace ec {

	void VulkanModel::create(VulkanModelCreateInfo& createInfo) {

		tinygltf::Model m = loadGltfModel(createInfo.filepath);
		m.buffers[0].data

	}

	void VulkanModel::destroy() {



	}

}