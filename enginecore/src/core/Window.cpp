#include "application.h"

#include <GLFW/glfw3.h>

namespace ec {

	void Window::create(WindowCreateInfo& createInfo) {

		EC_ASSERT(glfwInit());
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		m_nativWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title.c_str(), nullptr, nullptr);
		if (!m_nativWindow) {
			glfwTerminate();
			EC_ASSERT(m_nativWindow);
		}

	}

	void Window::update(bool& open) const
	{

		glfwPollEvents();
		open = glfwWindowShouldClose(m_nativWindow);

	}

	void Window::terminate()
	{
		glfwTerminate();
	}

	GLFWwindow* const Window::getNativWindow() const
	{
		return m_nativWindow;
	}

}
