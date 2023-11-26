#pragma once
#include "core/core.h"
#include "core/window.h"

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

	private:

		Window m_window;

	};


}