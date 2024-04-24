#include <glm/glm.hpp>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "rendering/renderer_data.h"
#include "mandelbrot.h"

glm::vec2 mpos;
MandelbrotSpec spec;

void drawMandelbrot()
{

    /*if (glfwGetMouseButton(renderer.getData().window.window->getNativWindow(), GLFW_MOUSE_BUTTON_3)) {

         double x = 0, y = 0;

         glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

         glm::vec2 diff = glm::vec2{ (float)x, (float)y } - mpos;
         diff *= -1.0f;
         glm::vec2 v = glm::vec2{ diff / glm::vec2{ spec.zoom * 100.0f }};
         spec.csX += v.x;
         spec.csY += v.y;


         mpos = glm::vec2{ (float)x, (float)y };

    }
    else {

        double x = 0, y = 0;

        glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

        mpos = glm::vec2{ (float)x, (float)y };

    }


    ImGui::Begin("Mandelbrot");

    ImGui::DragFloat("Scale", &spec.zoom, glm::pow(spec.zoom, 1.0f / 5.0f), 0.0f, 100000.0f);

    float value = std::pow(10.0f, glm::floor(glm::log(spec.zoom)));

    ImGui::DragFloat2("C0", (float*) &spec.csX, 1.0f / value);
    ImGui::DragFloat("Iterations", (float*) &spec.iterations, 1.0f, 0.0f, 10000.0f);

    ImGui::End();*/

}
