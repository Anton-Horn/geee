#include "application.h"
#include "log.h"

namespace ec {

	void createApplication(ApplicationCreateInfo& createInfo)
	{

		Application app = {};

		Log::create();

		createWindow(app.window, createInfo.windowCreateInfo);

		bool open = true;
		createInfo.createCallback(app);
		while (!open) {
			updateWindow(app.window, open);


			createInfo.updateCallback();

		}

		terminateWindow(app.window);

		Log::terminate();
	}

}