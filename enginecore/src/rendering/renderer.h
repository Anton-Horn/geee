#pragma once
#include "core/core.h"
#include "core/window.h"
#include "core/event_system.h"
#include "scene/scene.h"

namespace ec {

	struct RendererData;
	struct RendererSceneData;

	enum RendererFlags {

		RENDERER_NONE = 0,

		// if not set we also wait for the renderer to finish each frame
		// this is for the editor viewport synchronisation
		RENDERER_PRESENT_TO_SWAPCHAIN = 1,

		//can only be set if renderer does not present to swapchain
		RENDERER_DONT_AQUIRE_IMAGE = 2,

		RENDERER_CUSTOM_COMMAND_BUFFERS = 4,

		RENDERER_CUSTOM_QUEUE_SUBMIT = 8,

		RENDERER_ENABLE_VULKAN_INTERFACE = 16

		

	};

	struct VulkanSynchronizationControllerCreateInfo;

	struct RendererCreateInfo {

		const Window* window;
		uint32_t flags;

	};

	struct RendererBeginFrameInfo {

		bool* recreateSwapchain;

	};

	struct RendererPresentFrameInfo {

		bool* recreateSwapchain;

	};

	struct VulkanWindow;
	class VulkanContext;
	struct VulkanDataInterface;

	class Renderer {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(Renderer)

		void create(RendererCreateInfo& rendererCreateInfo);

		void beginFrame(RendererBeginFrameInfo& beginInfo);
		void drawFrame();
		void submitFrame();
		void presentFrame(RendererPresentFrameInfo& presentInfo);

		void destroy();

		void waitDeviceIdle();

		bool handleEvents(const Event& event);

		const VulkanDataInterface& getVulkanData() const;

	private:
		
		VulkanDataInterface* m_vulkanDataInterface;

		void recreate();
		RendererData* m_data;
		RendererSceneData* m_sceneData;
		uint32_t m_flags = RENDERER_NONE;
		bool m_recreateSwapchain = false;

		void createVulkanDataInterface();

	};

}

