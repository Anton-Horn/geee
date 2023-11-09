#include <vulkan/vulkan.h>
#include "vulkan_core.h"
#include "vulkan_utils.h"

namespace ec {

	VkBool32 VKAPI_CALL debugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			EC_ERROR(pCallbackData->pMessage);
		}
		else {
			EC_ERROR(pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	void VulkanContext::registerDebugCallback()
	{

		PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerExt;
		pfnCreateDebugUtilsMessengerExt = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_data.instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = debugReportCallback;

		VKA(pfnCreateDebugUtilsMessengerExt(m_data.instance, &createInfo, nullptr, &m_data.debugCallback));

	}

	void VulkanContext::createDevice(const std::vector<const char*>& deviceExtensions) {

		uint32_t physicalDevicesCount = 0;
		VKA(vkEnumeratePhysicalDevices(m_data.instance, &physicalDevicesCount, 0));

		VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[physicalDevicesCount];

		VKA(vkEnumeratePhysicalDevices(m_data.instance, &physicalDevicesCount, physicalDevices));

		VkPhysicalDevice discreteGPU = {};
		VkPhysicalDevice integratedGPU = {};
		VkPhysicalDevice mainGPU = {};


		for (uint32_t i = 0; i < physicalDevicesCount; i++) {

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				discreteGPU = physicalDevices[i];
				break;
			}
			else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {

				integratedGPU = physicalDevices[i];
			}
			EC_INFO(properties.deviceName);

		}

		if (discreteGPU) {
			mainGPU = discreteGPU;
		}
		else if (integratedGPU) {
			mainGPU = integratedGPU;
		}
		else {
			//No GPU found
			EC_ASSERT(false);
		}

		vkGetPhysicalDeviceProperties(mainGPU, &m_data.deviceProperties);
		m_data.physicalDevice = mainGPU;

		vkGetPhysicalDeviceMemoryProperties(mainGPU, &m_data.deviceMemoryProperties);

		uint32_t queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mainGPU, &queueFamilyPropertiesCount, nullptr);

		VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[queueFamilyPropertiesCount];
		vkGetPhysicalDeviceQueueFamilyProperties(mainGPU, &queueFamilyPropertiesCount, queueFamilyProperties);

		uint32_t graphicsQueueIndex = 0;

		for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {

			VkQueueFamilyProperties properties = queueFamilyProperties[i];

			if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				graphicsQueueIndex = i;
				break;
			}

		}

		m_data.queueFamilyIndex = graphicsQueueIndex;

		float queuePriorities[] = { 1.0f };

		VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
		queueCreateInfo.pQueuePriorities = queuePriorities;

		VkPhysicalDeviceFeatures features = {};

		VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.pEnabledFeatures = &features;

		VKA(vkCreateDevice(mainGPU, &createInfo, nullptr, &m_data.device));

		vkGetDeviceQueue(m_data.device, graphicsQueueIndex, 0, &m_data.queue);

		delete[] queueFamilyProperties;
		delete[] physicalDevices;


	}

	void VulkanContext::createInstance(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions) {

		VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		applicationInfo.pApplicationName = applicationName.c_str();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		applicationInfo.apiVersion = VK_API_VERSION_1_2;

		VkValidationFeatureEnableEXT enableValidationFeatures[] = {
			//VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
			VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
		};

		VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
		validationFeatures.enabledValidationFeatureCount = ARRAY_COUNT(enableValidationFeatures);
		validationFeatures.pEnabledValidationFeatures = enableValidationFeatures;

		VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pNext = &validationFeatures;
		createInfo.pApplicationInfo = &applicationInfo;
		createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledLayerCount = (uint32_t)layers.size();
		createInfo.ppEnabledLayerNames = layers.data();

		VKA(vkCreateInstance(&createInfo, 0, &m_data.instance));

		registerDebugCallback();

	}

	void VulkanContext::createDefaultVulkanContext(const std::string& applicationName, std::vector<const char*>& additionalWindowInstanceExtensions)
	{

		std::vector<const char*> enabledLayers;
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

		additionalWindowInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		additionalWindowInstanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

		std::vector<const char*> enabledDeviceExtensions;
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		createInstance(applicationName, enabledLayers, additionalWindowInstanceExtensions);
		createDevice(enabledDeviceExtensions);

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.device = m_data.device;
		allocatorCreateInfo.instance = m_data.instance;
		allocatorCreateInfo.physicalDevice = m_data.physicalDevice;
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		VKA(vmaCreateAllocator(&allocatorCreateInfo, &m_data.allocator));

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		m_data.generalDescriptorPool = createDesciptorPool(*this, 1000, poolSizes);
	}

	void VulkanContext::create(const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions) {


		createInstance(applicationName, layers, instanceExtensions);
		createDevice(deviceExtensions);

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.device = m_data.device;
		allocatorCreateInfo.instance = m_data.instance;
		allocatorCreateInfo.physicalDevice = m_data.physicalDevice;
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		VKA(vmaCreateAllocator(&allocatorCreateInfo, &m_data.allocator));

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		m_data.generalDescriptorPool = createDesciptorPool(*this, 1000, poolSizes);

	}

	void VulkanContext::destroy() {

		vmaDestroyAllocator(m_data.allocator);
		vkDestroyDevice(m_data.device, nullptr);
		vkDestroyInstance(m_data.instance, nullptr);

	}

	const VulkanContextData& VulkanContext::getData() const
	{
		return m_data;
	}

}





