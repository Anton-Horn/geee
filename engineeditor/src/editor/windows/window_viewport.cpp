#include <imgui.h>

#include "editor/editor.h"
#include "imgui_backend/imgui_impl_vulkan.h"

void ViewportWindow::create(SharedWindowData* sharedData, void* imageView, void* sampler) {
	
	m_renderImageDescriptorSet = ImGui_ImplVulkan_AddTexture((VkSampler)sampler, (VkImageView)imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_sharedData = sharedData;
}
void ViewportWindow::update() {

	ImGui::Begin("Viewport");

	

	ImGui::Image(m_renderImageDescriptorSet, ImGui::GetContentRegionAvail());

	ImGui::End();

}
void ViewportWindow::destroy() {
	ImGui_ImplVulkan_RemoveTexture( (VkDescriptorSet)m_renderImageDescriptorSet);
}