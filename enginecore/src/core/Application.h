#pragma once

#include <core/core.h>

namespace ec {

	class Application {

	public:

		Application() = default;
		~Application() = default;

		Application(Application& other) = delete;
		Application(Application&& other) = delete;

		Application& operator=(Application& other) = delete;
		Application& operator=(Application&& other) = delete;

		void create(void (*createCallback)(), void (*updateCallback)());

	private:


	};

}