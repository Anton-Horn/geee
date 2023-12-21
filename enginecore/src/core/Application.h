#pragma once
#include "core/core.h"
#include "core/window.h"
#include "core/job_system.h"
#include "core/event_system.h"

#include "rendering/renderer.h"

struct GLFWwindow;

namespace ec {

	class Application;

	struct ApplicationCreateInfo {

		std::function<void(Application&)> createCallback;
		std::function<void()> updateCallback;
		std::function<void()> terminateCallback;
		std::function<bool(const Event& event)> eventCallback;
		WindowCreateInfo windowCreateInfo;

	};

	class Application {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(Application)

		void create(ApplicationCreateInfo& createInfo);
		const Window& getWindow() const;

		const Renderer& getRenderer() const;
		Renderer& getRenderer();

		bool handleEvents(const Event& event);

		const Scene& getScene() const;

		Scene& getScene();

	private:

		void frame();

		Window m_window;
		JobSystem m_jobSystem;
		Renderer m_renderer;
		Scene m_scene;
		EventSystem m_eventSystem;

		bool m_windowShouldClose = false;
		std::function<void()> m_updateCallback;

	};


}