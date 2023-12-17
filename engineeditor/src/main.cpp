#include <imgui.h>

#include "ec.h"
#include "rendering/renderer.h"
#include "rendering/renderer_data.h"

#include "imgui_utils.h"
#include "editor/editor.h"

Editor editor;


void recreateApp() {
   /* imGuiUtilsDestroy();
    imGuiUtilsCreate();*/
}

ec::Application* app;

void createApp(ec::Application& app) { 

    ImGuiUtilsCreateInfo data;
    data.context = &app.getRenderer().getData().context;
    data.window = &app.getRenderer().getData().window;
    data.renderer = &app.getRenderer();
    
    imGuiUtilsCreate(data);

    EditorCreateInfo editorCreateInfo;
    editorCreateInfo.sampler = app.getRenderer().getData().presentRenderer.getData().sampler;
    editorCreateInfo.viewportImageView = app.getRenderer().getData().presentRenderer.getData().renderTarget.getImageView();
    editor.create(editorCreateInfo);
}

void updateApp() {

    imGuiUtilsBeginFrame();

    editor.update();

    imGuiUtilsEndFrame();

}

void terminateApp() {

    editor.destroy();
    imGuiUtilsDestroy();
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