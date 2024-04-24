#include <imgui.h>


#include "rendering/vulkan_impl/vulkan_utils.h"
#include "editor/editor.h"
#include "imgui_backend/imgui_impl_vulkan.h"

#include "rendering/renderer_data.h"
void ViewportWindow::create(SharedWindowData* sharedData, void* imageView) {
	
	m_sampler = (void*) ec::createSampler(*ec::Application::getInstance().getRenderer().getVulkanData().context);
	m_renderImageDescriptorSet = ImGui_ImplVulkan_AddTexture((VkSampler) m_sampler, (VkImageView)imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_sharedData = sharedData;
}
void ViewportWindow::update() {

	ImGui::Begin("Viewport");

	

	ImGui::Image(m_renderImageDescriptorSet, ImGui::GetContentRegionAvail());

	ImGui::End();

}
void ViewportWindow::destroy() {
	ImGui_ImplVulkan_RemoveTexture( (VkDescriptorSet)m_renderImageDescriptorSet);
	vkDestroySampler(ec::Application::getInstance().getRenderer().getVulkanData().context->getData().device, (VkSampler) m_sampler, nullptr);
}