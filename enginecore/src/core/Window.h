#pragma once

#include "core/core.h"

struct GLFWWindow;

struct WindowCreateInfo {

	uint32_t width;
	uint32_t height;

};


class Window {

public:

	void create(WindowCreateInfo& createInfo);
	void update();
	void terminate();

private:

	GLFWWindow* window;

};