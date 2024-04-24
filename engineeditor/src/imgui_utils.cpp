#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_backend/imgui_impl_glfw.h>
#include <imgui_backend/imgui_impl_vulkan.h>

#include <vulkan/vulkan.h>

#include "imgui_utils.h"
#include "core/application.h"
#include "rendering/vulkan_impl/vulkan_utils.h"
#include "rendering/vulkan_impl/vulkan_synchronisation.h"
#include "rendering/vulkan_impl/vulkan_renderer.h"
#include "rendering/renderer.h"
#include "rendering/vulkan_impl/vulkan_core.h"
#include "rendering/renderer_data.h"

struct ImGuiData {

    VkDescriptorPool descriptorPool;
    VkCommandBuffer commandBuffer;
    ec::VulkanRenderpass renderpass;
    std::vector<ec::VulkanFramebuffer> framebuffers;
    VkCommandPool commandPool;

    const ec::VulkanWindow* window;
    const ec::VulkanContext* context;
    const ec::Renderer* renderer;

    VkFence fence;
    VkSemaphore presentWaitSemaphore;
    VkSemaphore aquireSemaphore;

    bool recreateSwapchain = false;

};



ImGuiData imguiData;

void imGuiUtilsCreate()
{

    imguiData.renderer = &ec::Application::getInstance().getRenderer();
    imguiData.window = imguiData.renderer->getVulkanData().window;
    imguiData.context = imguiData.renderer->getVulkanData().context;

    imguiData.fence = (VkFence) imguiData.renderer->getVulkanData().addBeginFrameWaitingFence();
    imguiData.presentWaitSemaphore = (VkSemaphore)imguiData.renderer->getVulkanData().addPresentWaitSemaphore();

    imguiData.aquireSemaphore = ec::createSemaphore(*imguiData.renderer->getVulkanData().context);

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
    VKA(vkCreateDescriptorPool(imguiData.context->getData().device, &poolInfo, nullptr, &imguiData.descriptorPool));

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto/Roboto-Bold.ttf", 15.0f);

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(imguiData.window->window->getNativWindow(), true);

    const ec::VulkanContextData& contextData = imguiData.context->getData();

    VkAttachmentDescription colorAttachment = ec::createAttachment(1, imguiData.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
    std::vector<VkAttachmentReference> resolveAttachments = {};
    std::vector<VkAttachmentReference> inputAttachments = {};

    VkSubpassDescription subpassDescription = ec::createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

    imguiData.renderpass.create(*imguiData.context, { colorAttachment }, { subpassDescription });

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = contextData.instance;
    initInfo.PhysicalDevice = contextData.physicalDevice;
    initInfo.Device = contextData.device;
    initInfo.QueueFamily = contextData.queueFamilyIndex;
    initInfo.Queue = contextData.queue;
    initInfo.DescriptorPool = imguiData.descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = (uint32_t)imguiData.window->swapchain.getImages().size();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, imguiData.renderpass.getRenderpass());

    imguiData.commandPool = ec::createCommandPool(*imguiData.context);
    //imguiData.commandBuffer = (VkCommandBuffer) utilsData.renderer->getCommandBuffer(imguiData.commandPool);
    imguiData.commandBuffer = ec::allocateCommandBuffer(*imguiData.context, imguiData.commandPool);

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

    imguiData.framebuffers.resize(imguiData.window->swapchain.getImages().size());

    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].create(*imguiData.context, imguiData.renderpass, { &imguiData.window->swapchain.getImages()[i] });
    }

    imGuiStyle();

}

void imGuiUtilsRecreate()
{

    imguiData.renderpass.destroy(*imguiData.context);

    VkAttachmentDescription colorAttachment = ec::createAttachment(1, imguiData.window->swapchain.getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    std::vector<VkAttachmentReference> colorAttachmentReferences = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
    std::vector<VkAttachmentReference> resolveAttachments = {};
    std::vector<VkAttachmentReference> inputAttachments = {};

    VkSubpassDescription subpassDescription = ec::createSubpass(colorAttachmentReferences, resolveAttachments, inputAttachments);

    imguiData.renderpass.create(*imguiData.context, { colorAttachment }, { subpassDescription });

    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].destroy(*imguiData.context);
    }

    imguiData.framebuffers.resize(imguiData.window->swapchain.getImages().size());

    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].create(*imguiData.context, imguiData.renderpass, { &imguiData.window->swapchain.getImages()[i] });
    }

}

void imGuiUtilsDestroy()
{

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    vkDestroyDescriptorPool(imguiData.context->getData().device, imguiData.descriptorPool, nullptr);
    vkDestroyCommandPool(imguiData.context->getData().device, imguiData.commandPool, nullptr);
    for (uint32_t i = 0; i < imguiData.framebuffers.size(); i++) {
        imguiData.framebuffers[i].destroy(*imguiData.context);
    }
    imguiData.renderpass.destroy(*imguiData.context);
    vkDestroySemaphore(imguiData.context->getData().device, imguiData.aquireSemaphore, nullptr);

}

void imGuiUtilsBeginFrame()
{

    VKA(vkResetCommandPool(imguiData.context->getData().device, imguiData.commandPool, 0));

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VKA(vkBeginCommandBuffer(imguiData.commandBuffer, &beginInfo));

    imguiData.renderer->getVulkanData().aquireNextSwapchainImage(imguiData.aquireSemaphore, imguiData.recreateSwapchain);

    if (imguiData.recreateSwapchain) return;

    VkRenderPassBeginInfo renderpassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderpassBeginInfo.renderPass = imguiData.renderpass.getRenderpass();

    renderpassBeginInfo.framebuffer = imguiData.framebuffers[imguiData.window->swapchain.getCurrentIndex()].getFramebuffer();
    renderpassBeginInfo.renderArea = { 0,0, imguiData.window->swapchain.getWidth(), imguiData.window->swapchain.getHeight() };
    renderpassBeginInfo.clearValueCount = 1;
    VkClearValue clearValue = { 0.1f, 0.1f, 0.102f, 1.0f };
    renderpassBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(imguiData.commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void imGuiUtilsEndFrame()
{

    if (imguiData.recreateSwapchain) {
        ec::Application::getInstance().getEventSystem().triggerEventDeferred({ ec::EventType::ApplicationRecreateEvent, nullptr, 0 });
        return;
    }

    ImGui::Render();
    ImDrawData* mainDrawData = ImGui::GetDrawData();

    ImGui_ImplVulkan_RenderDrawData(mainDrawData, imguiData.commandBuffer);

    vkCmdEndRenderPass(imguiData.commandBuffer);
    vkEndCommandBuffer(imguiData.commandBuffer);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &imguiData.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &imguiData.presentWaitSemaphore;
   
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imguiData.aquireSemaphore;

    VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitMask;
     
    imguiData.renderer->getVulkanData().submitFrameSynchronized({ imguiData.commandBuffer }, false, imguiData.fence, imguiData.presentWaitSemaphore, imguiData.aquireSemaphore);

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    
}

void imGuiStyle()
{
    // Photoshop style by Derydoca from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 2.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 2.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 13.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabMinSize = 7.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 1.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317f, 0.2745098173618317f, 0.2745098173618317f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3910000026226044f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6700000166893005f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288f, 0.5843137502670288f, 0.5843137502670288f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
    
}
