#include <imgui.h>
#include <imgui_backend/imgui_impl_glfw.h>
#include <imgui_backend/imgui_impl_vulkan.h>

#include "ec.h"
#include "rendering/renderer.h"
#include "rendering/vulkan_impl/vulkan_renderer.h"
#include "rendering/vulkan_impl/vulkan_core.h"
#include "rendering/vulkan_impl/vulkan_synchronisation.h"
#include "rendering/vulkan_impl/vulkan_utils.h"

#include "bezier_spline.h"
#include <GLFW/glfw3.h>
namespace ec {

    struct RendererData {
        VulkanContext context;
        VulkanWindow window;
        VulkanSynchronisationController synController;

        VulkanQuadRenderer quadRenderer;
        VulkanBezierRenderer bezierRenderer;
        VulkanGoochRenderer goochRenderer;
        VulkanMandelbrotRenderer mandelbrotRenderer;

        VulkanModel model;
        VulkanImage image;

        VkCommandBuffer additionalCommandBuffer;
        VkCommandPool commandPool;
        VkDescriptorPool rpfDescriptorPool;

        MandelbrotSpec mandelbrotSpec;

    };

}

ec::Renderer renderer;

struct ImGuiData {

    VkDescriptorPool descriptorPool;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    ec::VulkanRenderpass renderpass;
    std::vector<ec::VulkanFramebuffer> framebuffers;

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
    VKA(vkCreateDescriptorPool(renderer.getData().context.getData().device, &poolInfo, nullptr, &imguiData.descriptorPool));
  

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
    initInfo.DescriptorPool = imguiData.descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = (uint32_t)renderer.getData().window.swapchain.getImages().size();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, renderer.getData().quadRenderer.getData().renderpass.getRenderpass());

    imguiData.commandPool = ec::createCommandPool(renderer.getData().context);
    imguiData.commandBuffer = ec::allocateCommandBuffer(renderer.getData().context, imguiData.commandPool);

    VKA(vkResetCommandPool(contextData.device, imguiData.commandPool, 0));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VKA(vkBeginCommandBuffer(imguiData.commandBuffer, &beginInfo));

    ImGui_ImplVulkan_CreateFontsTexture(imguiData.commandBuffer);

    VKA(vkEndCommandBuffer(imguiData.commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &imguiData.commandBuffer;

    VKA(vkQueueSubmit(contextData.queue, 1, &submitInfo, VK_NULL_HANDLE));

    VKA(vkDeviceWaitIdle(contextData.device));
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  

    VkAttachmentDescription colorAttachment = ec::createAttachment(1, renderer.getData().window.swapchain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
    std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
    std::vector<VkAttachmentReference> resolveAttachments = {};
    std::vector<VkAttachmentReference> inputAttachments = {};
    VkSubpassDescription subpassDescription = ec::createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

    imguiData.renderpass.create(renderer.getData().context, {colorAttachment}, {subpassDescription});

    imguiData.framebuffers.resize(renderer.getData().window.swapchain.getImages().size());

    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].create(renderer.getData().context, imguiData.renderpass, {&renderer.getData().window.swapchain.getImages()[i]});
    }
    renderer.getData().additionalCommandBuffer = imguiData.commandBuffer;

}

void destroyImgui() {

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    vkDestroyDescriptorPool(renderer.getData().context.getData().device, imguiData.descriptorPool, nullptr);
    vkDestroyCommandPool(renderer.getData().context.getData().device, imguiData.commandPool, nullptr);
    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].destroy(renderer.getData().context);
    }
    imguiData.renderpass.destroy(renderer.getData().context);

}

BezierSpline spline;

void renderCurves() {

    double x = 0, y = 0;

    glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

    x -= renderer.getData().window.swapchain.getWidth() / 2.0f;
    y -= renderer.getData().window.swapchain.getHeight() / 2.0f;
    y *= -1;

    bool mdown = glfwGetMouseButton(renderer.getData().window.window->getNativWindow(), GLFW_MOUSE_BUTTON_1);

    spline.handleInput((float)x, (float)y, mdown);
    spline.draw(renderer);

}

glm::vec2 mpos;

void renderImGui(){


    vkResetCommandPool(renderer.getData().context.getData().device, imguiData.commandPool, 0);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VKA(vkBeginCommandBuffer(imguiData.commandBuffer, &beginInfo));

    VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderpassBeginInfo.renderPass = imguiData.renderpass.getRenderpass();
    renderpassBeginInfo.framebuffer = imguiData.framebuffers[renderer.getData().window.swapchain.getCurrentIndex()].getFramebuffer();
    renderpassBeginInfo.renderArea = { 0,0, renderer.getData().window.swapchain.getWidth(), renderer.getData().window.swapchain.getHeight() };
    renderpassBeginInfo.clearValueCount = 0;
    renderpassBeginInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(imguiData.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


    if (glfwGetMouseButton(renderer.getData().window.window->getNativWindow(), GLFW_MOUSE_BUTTON_3)) {

         double x = 0, y = 0;

         glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

         glm::vec2 diff = glm::vec2{ (float)x, (float)y } - mpos;
         diff *= -1.0f;
         glm::vec2 v = glm::vec2{ diff / glm::vec2{ renderer.getData().mandelbrotSpec.zoom * 100.0f }};
         renderer.getData().mandelbrotSpec.csX += v.x;
         renderer.getData().mandelbrotSpec.csY += v.y;


         mpos = glm::vec2{ (float)x, (float)y };

    }
    else {

        double x = 0, y = 0;

        glfwGetCursorPos(renderer.getData().window.window->getNativWindow(), &x, &y);

        mpos = glm::vec2{ (float)x, (float)y };

    }
    

    ImGui::Begin("Mandelbrot");
    
    ImGui::DragFloat("Scale", &renderer.getData().mandelbrotSpec.zoom, glm::pow(renderer.getData().mandelbrotSpec.zoom, 1.0f / 5.0f), 0.0f, 100000.0f);

    float value = std::pow(10.0f, glm::floor(glm::log(renderer.getData().mandelbrotSpec.zoom)));

    ImGui::DragFloat2("C0", (float*)&renderer.getData().mandelbrotSpec.csX, 1.0f / value);
    ImGui::DragFloat("Iterations", (float*) &renderer.getData().mandelbrotSpec.iterations, 1.0f, 0.0f, 10000.0f);

    ImGui::End();

    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();

    ImGui_ImplVulkan_RenderDrawData(main_draw_data, imguiData.commandBuffer);

    vkCmdEndRenderPass(imguiData.commandBuffer);
    vkEndCommandBuffer(imguiData.commandBuffer);

}

void recreateImGui() {

    destroyImgui();
    initImGui();

}

void create(ec::Application& app) { 

    spline.create({ -100.0f, 0.0f, 0.0f }, { 200.0f, 200.0f, 0.0f }, { 50.0f, 50.0f, 0.0f }, { 100.0f, 100.0f, 0.0f });
    spline.addCurve({ 250.0f, 250.0f, 0.0f }, { 300.0f, 300.0f, 0.0f }, { 350.0f, 350.0f, 0.0f });
    spline.addCurve({ 250.0f, 250.0f, 0.0f }, { 300.0f, 300.0f, 0.0f }, { 350.0f, 350.0f, 0.0f });
    ec::RendererCreateInfo createInfo;
    createInfo.window = &app.getWindow();

    ec::RendererCallbacks callbacks;
   // callbacks.bezierRendererDrawCallback = renderCurves;
    callbacks.drawCallback = renderImGui;
    callbacks.recreateCallback = recreateImGui;
    createInfo.callbacks = std::move(callbacks);
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

void terminateApplication() {

   
    renderer.getData().synController.waitDeviceIdle(renderer.getData().context);
    destroyImgui();
    renderer.destroy();
}


int main() {




	ec::ApplicationCreateInfo createInfo;
	createInfo.createCallback = create;
	createInfo.updateCallback = update;
    createInfo.terminateCallback = terminateApplication;
	createInfo.windowCreateInfo = { 1280, 720, "editor" };

	ec::Application app;
	app.create(createInfo);

	return 0;

}