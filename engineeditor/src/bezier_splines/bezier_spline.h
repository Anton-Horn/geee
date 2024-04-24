#pragma once
#include <glm/glm.hpp>
#include <array>

#include "core/core.h"
#include "rendering/renderer.h"

class BezierSpline {

public:

	void create(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2);
	void addCurve(const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2);
	void handleInput(float mx, float my, bool mdown);
	void draw();

private:

	std::vector<glm::vec3> m_points;
	int m_selectedPointIndex = -1;

};