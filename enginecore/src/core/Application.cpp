#include "application.h"
#include "log.h"

namespace ec {

	void Application::create(ApplicationCreateInfo& createInfo)
	{

		Log::create();

		m_window.create(createInfo.windowCreateInfo);

		bool close = false;
		createInfo.createCallback(*this);
		while (!close) {
			m_window.update(close);

			createInfo.updateCallback();
		}

		createInfo.terminateCallback();

		m_window.terminate();

		Log::terminate();
	}

	const Window& Application::getWindow() const
	{
		return m_window;
	}

}