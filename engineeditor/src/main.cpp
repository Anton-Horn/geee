#include "ec.h"
#include "rendering/renderer.h"

ec::Renderer renderer;

void create(ec::Application& app) {

	renderer.init(app.getWindow());

}

void update(){
	
	renderer.draw();

}


int main() {


	ec::ApplicationCreateInfo createInfo;
	createInfo.createCallback = create;
	createInfo.updateCallback = update;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };

	ec::Application app;
	app.create(createInfo);
	renderer.destroy();
	return 0;

}