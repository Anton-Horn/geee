#include <imgui.h>

#include "ec.h"
#include "rendering/renderer.h"
#include "rendering/renderer_data.h"

#include "imgui_utils.h"
#include "editor/editor.h"

Editor editor;

ec::Application* application;

void createApp(ec::Application& app) { 

    ImGuiUtilsCreateInfo data;
    data.context = &app.getRenderer().getData().context;
    data.window = &app.getRenderer().getData().window;
    data.renderer = &app.getRenderer();
    
    imGuiUtilsCreate(data);

    EditorCreateInfo editorCreateInfo;
    editorCreateInfo.sampler = app.getRenderer().getData().presentRenderer.getData().sampler;
    editorCreateInfo.viewportImageView = app.getRenderer().getData().presentRenderer.getData().renderTarget.getImageView();
    editorCreateInfo.scene = &app.getScene();
    editor.create(editorCreateInfo);


    application = &app;
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

bool handleEvents(const ec::Event& event) {

    if (event.eventType == ec::EventType::ApplicationRecreateEvent) {
        editor.destroy();
        imGuiUtilsDestroy();
        ImGuiUtilsCreateInfo data;
        data.context = &application->getRenderer().getData().context;
        data.window = &application->getRenderer().getData().window;
        data.renderer = &application->getRenderer();
        imGuiUtilsCreate(data);

        EditorCreateInfo editorCreateInfo;
        editorCreateInfo.sampler = application->getRenderer().getData().presentRenderer.getData().sampler;
        editorCreateInfo.viewportImageView = application->getRenderer().getData().presentRenderer.getData().renderTarget.getImageView();
        editorCreateInfo.scene = &application->getScene();
        editor.create(editorCreateInfo);
    }

    return false;
}


int main() {

	ec::ApplicationCreateInfo createInfo;
	createInfo.createCallback = createApp;
	createInfo.updateCallback = updateApp;
    createInfo.terminateCallback = terminateApp;
    createInfo.eventCallback = handleEvents;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };

	ec::Application app;
	app.create(createInfo);

	return 0;
}