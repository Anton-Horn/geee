#pragma once
#include "core/core.h"

struct GLFWwindow;

namespace ec {

	struct WindowCreateInfo {

		uint32_t width;
		uint32_t height;
		std::string title;

	};

	struct Window {

		GLFWwindow* nativWindow;

	};

	struct Application;

	struct ApplicationCreateInfo {

		std::function<void(Application&)> createCallback;
		std::function<void()> updateCallback;
		WindowCreateInfo windowCreateInfo;

	};

	struct Application {

		Window window;

	};

	void createApplication(ApplicationCreateInfo& createInfo);

	void createWindow(Window& window, WindowCreateInfo& createInfo);
	void updateWindow(Window& window, bool& open);
	void terminateWindow(Window& window);

}