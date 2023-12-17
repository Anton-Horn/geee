#pragma once
#include "core/core.h"
#include "core/window.h"
#include "scene/scene.h"


namespace ec {

	struct RendererData;
	struct RendererSceneData;

	enum RendererFlags {

		RENDERER_NONE = 0,

		// if not set we also wait for the renderer to finish each frame
		// this is for the editor viewport synchronisation
		RENDERER_PRESENT_TO_SWAPCHAIN = 1

	};

	struct RendererCreateInfo {

		const Window* window;
		RendererFlags flags;
		
	};

	class Renderer {

	public:

		void create(RendererCreateInfo& rendererCreateInfo);
		void setSceneData(const Scene& scene);

		void beginFrame();
		void drawFrame();
		void submitFrame();
		void presentFrame();

		void destroy();

		void waitDeviceIdle();

		//Should only be called if in RendererCreateInfo, presentToSwapchain = false
		void* getPresentImageHandle();
		//Should only be called if in RendererCreateInfo, presentToSwapchain = false
		void* getPresentImageView();

		void* getCommandBuffer(void* commandPool = nullptr);

		Renderer();
		~Renderer();

		const RendererData& getData() const;
		RendererData& getData();

	private:

		void recreate();
		
		std::unique_ptr<RendererData> m_data;
		std::unique_ptr<RendererSceneData> m_sceneData;
		RendererFlags m_flags = RENDERER_NONE;
	};

}

