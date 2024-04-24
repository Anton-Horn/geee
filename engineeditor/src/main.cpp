#include <imgui.h>

#include "ec.h"
#include "rendering/renderer.h"
#include "rendering/renderer_data.h"

#include "imgui_utils.h"
#include "editor/editor.h"

Editor editor;

void createApp(ec::Application& app) { 
    
    imGuiUtilsCreate();

    EditorCreateInfo editorCreateInfo;
    editorCreateInfo.viewportImageView = app.getRenderer().getVulkanData().getPresentImageView();
    editor.create(editorCreateInfo);
    
}

void updateApp() {

    imGuiUtilsBeginFrame();

    editor.update();

    imGuiUtilsEndFrame();

}

void synchronizedUpdate() {
    editor.synchronizedUpdate();
}

void terminateApp() {

    editor.destroy();
    imGuiUtilsDestroy();
}

bool handleEvents(const ec::Event& event) {

    if (event.eventType == ec::EventType::ApplicationRecreateEvent) {
        editor.destroy();
        imGuiUtilsRecreate();

        EditorCreateInfo editorCreateInfo;
        editorCreateInfo.viewportImageView = ec::Application::getInstance().getRenderer().getVulkanData().getPresentImageView();
        editor.create(editorCreateInfo);
    }

    return false;
}

int main() {

    ec::Application app;

    ec::ApplicationCreateInfo createInfo = {};
	createInfo.createCallback = createApp;
	createInfo.updateCallback = updateApp;
    createInfo.terminateCallback = terminateApp;
    createInfo.eventCallback = handleEvents;
    createInfo.synchronizedUpdateCallback = synchronizedUpdate;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };
    createInfo.customRendererCreateInfo.flags = ec::RENDERER_DONT_AQUIRE_IMAGE |ec::RENDERER_CUSTOM_COMMAND_BUFFERS |ec::RENDERER_CUSTOM_QUEUE_SUBMIT | ec::RENDERER_ENABLE_VULKAN_INTERFACE;
    createInfo.flags = ec::APPLICATION_USE_CUSTOM_RENDERER_CREATE_INFO;
    createInfo.app = &app;

	app.create(createInfo);

	return 0;
}