#include <imgui.h>
#include <imgui_backend/imgui_impl_glfw.h>
#include <imgui_backend/imgui_impl_vulkan.h>

#include "ec.h"
#include "rendering/renderer.h"
#include "rendering/vulkan_impl/vulkan_renderer.h"
#include "rendering/vulkan_impl/vulkan_core.h"

#include "bezier_spline.h"
namespace ec {

    struct RendererData {

        VulkanContext context;
        VulkanWindow window;
        VulkanSynchronisationController synController;
        VulkanQuadRenderer quadRenderer;
        VulkanBezierRenderer bezierRenderer;

        VulkanImage image;

    };

}

ec::Renderer renderer;

struct ImGuiData {

    VkDescriptorPool imguiDescriptorPool;

};

ImGuiData imguiData;

void initImGui(){

    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * ARRAY_COUNT(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)ARRAY_COUNT(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    VKA(vkCreateDescriptorPool(renderer.getData().context.getData().device, &poolInfo, nullptr, &imguiData.imguiDescriptorPool));
  

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(renderer.getData().window.window->getNativWindow(), true);

    const ec::VulkanContextData& contextData = renderer.getData().context.getData();

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = contextData.instance;
    initInfo.PhysicalDevice = contextData.physicalDevice;
    initInfo.Device = contextData.device;
    initInfo.QueueFamily = contextData.queueFamilyIndex;
    initInfo.Queue = contextData.queue;
    initInfo.DescriptorPool = imguiData.imguiDescriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = renderer.getData().window.swapchain.getImages().size();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, renderer.getData().quadRenderer.getData().renderpass.getRenderpass());

    // Use any command queue
    VkCommandPool commandPool = renderer.getData().quadRenderer.getData().commandPool;
    VkCommandBuffer commandBuffer = renderer.getData().quadRenderer.getData().commandBuffer;

    VKA(vkResetCommandPool(contextData.device, commandPool, 0));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VKA(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    VKA(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VKA(vkQueueSubmit(contextData.queue, 1, &submitInfo, VK_NULL_HANDLE));

    VKA(vkDeviceWaitIdle(contextData.device));
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    
}

BezierSpline spline;

void renderCurves() {

    double x = 0, y = 0;

    glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

    x -= renderer.getData().window.swapchain.getWidth() / 2.0f;
    y -= renderer.getData().window.swapchain.getHeight() / 2.0f;
    y *= -1;

    bool mdown = glfwGetMouseButton(renderer.getData().window.window->getNativWindow(), GLFW_MOUSE_BUTTON_1);

    spline.handleInput(x, y, mdown);
    spline.draw(renderer);

}

void renderImGui(){

    ImGui::Begin("Curve");



    ImGui::End();


    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();

    ImGui_ImplVulkan_RenderDrawData(main_draw_data, renderer.getData().quadRenderer.getData().commandBuffer);

}

void create(ec::Application& app) { 

    spline.create({ -100.0f, 0.0f, 0.0f }, { 200.0f, 200.0f, 0.0f }, { 50.0f, 50.0f, 0.0f }, { 100.0f, 100.0f, 0.0f });
    spline.addCurve({ 250.0f, 250.0f, 0.0f }, { 300.0f, 300.0f, 0.0f }, { 350.0f, 350.0f, 0.0f });
    spline.addCurve({ 250.0f, 250.0f, 0.0f }, { 300.0f, 300.0f, 0.0f }, { 350.0f, 350.0f, 0.0f });
    ec::RendererCreateInfo createInfo = { &app.getWindow(), renderImGui, renderCurves };

    renderer.init(createInfo);
    initImGui();
}

void update(){
	
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

	renderer.draw();

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

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