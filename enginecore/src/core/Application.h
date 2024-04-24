#pragma once
#include "core/core.h"
#include "core/window.h"
#include "core/job_system.h"
#include "core/event_system.h"

#include "asset/asset.h"

#include "rendering/renderer.h"

struct GLFWwindow;

namespace ec {

	class Application;

	enum ApplicationFlags {
		
		APPLICATION_USE_CUSTOM_RENDERER_CREATE_INFO = 1

	};

	class Application;

	struct ApplicationCreateInfo {

		std::function<void(Application&)> createCallback;
		std::function<void()> updateCallback;
		std::function<void()> synchronizedUpdateCallback;
		std::function<void()> terminateCallback;
		std::function<bool(const Event& event)> eventCallback;
		WindowCreateInfo windowCreateInfo;

		//window gets overwritten by application
		RendererCreateInfo customRendererCreateInfo;
		uint32_t flags;

		Application* app;

	};

	class Application {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(Application)

		void create(ApplicationCreateInfo& createInfo);

		const Window& getWindow() const;

		Renderer& getRenderer();

		EventSystem& getEventSystem();

		AssetManager& getAssetManager();

		bool handleEvents(const Event& event);

		static Application& getInstance();

	private:

		void frame();

		Window m_window;
		JobSystem m_jobSystem;
		Renderer m_renderer;
		EventSystem m_eventSystem;
		AssetManager m_assetManager;

		bool m_windowShouldClose = false;
		std::function<void()> m_updateCallback;
		std::function<void()> m_synchronizedUpdateCallback;

		static Application* s_app;

	};


}