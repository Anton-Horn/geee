#include "event_system.h"

namespace ec {

	void EventSystem::addEventListener(std::function<bool(const Event&)> listener)
	{
		m_eventListeners.push_back(std::move(listener));
	}

	void EventSystem::triggerEvent(const Event& event, bool all)
	{

		for (auto& listener : m_eventListeners) {

			bool result = listener(event);
			if (result && !all) break;

		}

	}

}
