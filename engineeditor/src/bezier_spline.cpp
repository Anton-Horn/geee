#include "bezier_spline.h"


void BezierSpline::create(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2)
{
    m_points.emplace_back(p1);
    m_points.emplace_back(c1);
    m_points.emplace_back(c2);
    m_points.emplace_back(p2);
}

void BezierSpline::addCurve(const glm::vec3& p2, const glm::vec3& c1, const glm::vec3& c2) {
	m_points.emplace_back(c1);
    m_points.emplace_back(c2);
    m_points.emplace_back(p2);
}
void BezierSpline::handleInput(float mx, float my, bool mdown) {


    if (mdown && m_selectedPointIndex == -1) {

        float min_length = FLT_MAX;
        for (uint32_t i = 0; i < m_points.size(); i++) {


            float l = glm::length(m_points[i] - glm::vec3{mx, my, 0.0f});

            if (glm::min(l, min_length) == l && l < 50.0f) {
                min_length = l;
                
                m_selectedPointIndex = i;

            }

        }

    }
    else {
        m_selectedPointIndex = -1;
    }


    if (m_selectedPointIndex != -1) {

        m_points[m_selectedPointIndex] = { mx, my, 0.0f };

    }

}

void BezierSpline::draw(ec::Renderer& renderer)
{

    renderer.drawBezierCurve(*(ec::vec3*)&m_points[0], *(ec::vec3*)&m_points[3], *(ec::vec3*)&m_points[1], *(ec::vec3*)&m_points[2], { 1.0f, 1.0f, 1.0f, 1.0f });

    for (uint32_t i = 3; i < m_points.size(); i += 3) {
   
        if (i + 3 < m_points.size()) 
        renderer.drawBezierCurve(*(ec::vec3*)&m_points[0 + i], *(ec::vec3*)&m_points[3 + i], *(ec::vec3*)&m_points[1 + i], *(ec::vec3*)&m_points[2 + i], { 1.0f, 1.0f, 1.0f, 1.0f });

    }

}
