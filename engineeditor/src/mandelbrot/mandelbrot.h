#pragma once
#include "rendering/renderer.h"

struct MandelbrotSpec {

	float csX;
	float csY;
	float iterations;
	float zoom;

};

void drawMandelbrot();