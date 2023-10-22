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

	void registerDebugCallback(VulkanContext& context)
	{

		PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerExt;
		pfnCreateDebugUtilsMessengerExt = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = debugReportCallback;

		VKA(pfnCreateDebugUtilsMessengerExt(context.instance, &createInfo, nullptr, &context.debugCallback));

	}

	void createDevice(VulkanContext& context, const std::vector<const char*>& deviceExtensions) {

		uint32_t physicalDevicesCount = 0;
		VKA(vkEnumeratePhysicalDevices(context.instance, &physicalDevicesCount, 0));

		VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[physicalDevicesCount];

		VKA(vkEnumeratePhysicalDevices(context.instance, &physicalDevicesCount, physicalDevices));

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

		vkGetPhysicalDeviceProperties(mainGPU, &context.deviceProperties);
		context.physicalDevice = mainGPU;

		vkGetPhysicalDeviceMemoryProperties(mainGPU, &context.deviceMemoryProperties);

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

		context.queueFamilyIndex = graphicsQueueIndex;

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

		VKA(vkCreateDevice(mainGPU, &createInfo, nullptr, &context.device));

		vkGetDeviceQueue(context.device, graphicsQueueIndex, 0, &context.queue);

		delete[] queueFamilyProperties;
		delete[] physicalDevices;


	}

	void createInstance(VulkanContext& context, const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions) {

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

		VKA(vkCreateInstance(&createInfo, 0, &context.instance));

		registerDebugCallback(context);

	}

	void createDefaultVulkanContext(VulkanContext& context, const std::string& applicationName, std::vector<const char*>& additionalWindowInstanceExtensions)
	{

		std::vector<const char*> enabledLayers;
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

		additionalWindowInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		additionalWindowInstanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

		std::vector<const char*> enabledDeviceExtensions;
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		createInstance(context, applicationName, enabledLayers, additionalWindowInstanceExtensions);
		createDevice(context, enabledDeviceExtensions);

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.device = context.device;
		allocatorCreateInfo.instance = context.instance;
		allocatorCreateInfo.physicalDevice = context.physicalDevice;
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		VKA(vmaCreateAllocator(&allocatorCreateInfo, &context.allocator));

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

		context.generalDescriptorPool = createDesciptorPool(context, 1000, poolSizes);
	}

	void createVulkanContext(VulkanContext& context, const std::string& applicationName, const std::vector<const char*>& layers, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions) {


		createInstance(context, applicationName, layers, instanceExtensions);
		createDevice(context, deviceExtensions);

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.device = context.device;
		allocatorCreateInfo.instance = context.instance;
		allocatorCreateInfo.physicalDevice = context.physicalDevice;
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		VKA(vmaCreateAllocator(&allocatorCreateInfo, &context.allocator));

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

		context.generalDescriptorPool = createDesciptorPool(context, 1000, poolSizes);

	}

	void destroyVulkanContext(VulkanContext& context) {

		vmaDestroyAllocator(context.allocator);
		vkDestroyDevice(context.device, nullptr);
		vkDestroyInstance(context.instance, nullptr);

	}

}




