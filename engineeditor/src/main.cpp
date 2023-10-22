#include "ec.h"

void create(ec::Application& app) {
	
}

void update(){
	
}

int main() {

	ec::ApplicationCreateInfo createInfo;
	createInfo.createCallback = create;
	createInfo.updateCallback = update;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };

	ec::createApplication(&createInfo);

	return 0;

}