#pragma once

#include "rendering/renderer.h"

namespace ec {
	class VulkanContext;
	struct VulkanWindow;
}

struct ImGuiUtilsCreateInfo {

	const ec::VulkanWindow* window;
	ec::VulkanContext* context;
	ec::Renderer* renderer;

};

void imGuiUtilsCreate(ImGuiUtilsCreateInfo& createInfo);
void imGuiUtilsDestroy();
void imGuiUtilsBeginFrame();
void imGuiUtilsEndFrame();

void imGuiStyle();
