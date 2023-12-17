#pragma once
#include "core/core.h"
#include "core/window.h"
#include "core/job_system.h"

#include "rendering/renderer.h"

struct GLFWwindow;

namespace ec {

	class Application;

	struct ApplicationCreateInfo {

		std::function<void(Application&)> createCallback;
		std::function<void()> updateCallback;
		std::function<void()> terminateCallback;
		WindowCreateInfo windowCreateInfo;

	};

	class Application {

	public:

		Application() = default;
		~Application() = default;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		void create(ApplicationCreateInfo& createInfo);
		const Window& getWindow() const;

		const Renderer& getRenderer() const;
		Renderer& getRenderer();

	private:

		void frame();

		Window m_window;
		JobSystem m_jobSystem;
		Renderer m_renderer;
		Scene m_scene;

		bool m_windowShouldClose = false;
		std::function<void()> m_updateCallback;

	};


}