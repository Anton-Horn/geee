#include "application.h"

#include <GLFW/glfw3.h>

namespace ec {

	void createWindow(Window& window, WindowCreateInfo& createInfo) {

		EC_ASSERT(glfwInit());
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		window.nativWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title.c_str(), nullptr, nullptr);
		if (!window.nativWindow) {
			glfwTerminate();
			EC_ASSERT(window.nativWindow);
		}

	}

	void updateWindow(Window& window, bool& open)
	{

		glfwPollEvents();
		open = glfwWindowShouldClose(window.nativWindow);

	}

	void terminateWindow(Window& window)
	{
		glfwTerminate();
	}

}
