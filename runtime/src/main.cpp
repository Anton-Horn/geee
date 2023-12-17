#include "ec.h"

void createApp(ec::Application& app) {
	


}

void updateApp() {


}


void terminateApp() {



}

int main() {

	ec::ApplicationCreateInfo createInfo;
	createInfo.createCallback = createApp;
	createInfo.updateCallback = updateApp;
	createInfo.terminateCallback = terminateApp;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };

	ec::Application app;
	app.create(createInfo);

	return 0;
}