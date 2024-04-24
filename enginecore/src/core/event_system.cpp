#include "event_system.h"

namespace ec {

	void EventSystem::addEventListener(std::function<bool(const Event&)> listener)
	{
		m_eventListeners.push_back(std::move(listener));
		EC_LOG("Event System: added an event listener");
	}

	void EventSystem::triggerEvent(const Event& event, bool all)
	{

		for (auto& listener : m_eventListeners) {

			bool result = listener(event);
			if (result && !all) break;

		}
	}

	void EventSystem::triggerEventDeferred(const Event& event, bool all)
	{

		std::lock_guard<std::mutex> lock(m_mutex);
		m_deferredEventQueue.push_back(std::make_pair(event, all));

	}

	void EventSystem::handleDeferredEvents()
	{

		for (auto& pair : m_deferredEventQueue) {
			triggerEvent(std::get<0>(pair), std::get<1>(pair));
		}
		m_deferredEventQueue.clear();

	}

	void EventSystem::create()
	{
		m_deferredEventQueue.reserve(10);
	}

	void EventSystem::destroy()
	{
	}

}
