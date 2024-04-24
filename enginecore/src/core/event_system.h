#pragma once
#include <vector>
#include <mutex>

#include "core/core.h"


namespace ec {

	enum class EventType {

		ApplicationRecreateEvent

	};

	struct Event {

		EventType eventType;

		void* eventData;
		uint32_t eventDataSize;

	};

	class EventSystem {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(EventSystem)

		void addEventListener(std::function<bool(const Event&)> listener);
		void triggerEvent(const Event& event, bool all = true);

		// thread safe
		void triggerEventDeferred(const Event& event, bool all = true);

		//should only be called on the main thread while synchronized
		void handleDeferredEvents();

		void create();
		void destroy();

	private:

		std::vector<std::function<bool(const Event&)>> m_eventListeners;
		std::vector<std::pair<Event, bool>> m_deferredEventQueue;
		std::mutex m_mutex;

	};

}