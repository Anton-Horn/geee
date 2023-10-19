#include "Application.h"

#include "log.h"

namespace ec {


	void Application::create(void (*createCallback)(), void (*updateCallback)())
	{
		
		Log::create();
		
		bool open = true;
		createCallback();
		while (open) {

			updateCallback();

		}
		Log::terminate();
	}

}