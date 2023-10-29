#pragma once

#include "core/core.h"

struct GLFWwindow;

namespace ec {

	struct WindowCreateInfo {

		uint32_t width;
		uint32_t height;
		std::string title;

	};

	class Window {

	public:

		Window() = default;
		~Window() = default;

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;


		void create(WindowCreateInfo& createInfo);
		void update(bool& open) const;
		void terminate();

		GLFWwindow* const getNativWindow() const;

	private:

		GLFWwindow* m_nativWindow;

	};

}