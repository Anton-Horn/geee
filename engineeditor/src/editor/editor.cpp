#include <imgui.h>
#include <imgui_internal.h>
#include "editor.h"

void Editor::create(EditorCreateInfo& createInfo)
{
	m_sceneHierachyPanel.create(&m_sharedData);
	m_inspecorWindow.create(&m_sharedData);
	m_viewportWindow.create(&m_sharedData, createInfo.viewportImageView, createInfo.sampler);
}

void Editor::destroy() {

	m_sceneHierachyPanel.destroy();
	m_inspecorWindow.destroy();
	m_viewportWindow.destroy();

}
const SharedWindowData& Editor::getSharedData() const
{
	return m_sharedData;
}
void Editor::update() {

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Debug")) {

			if (ImGui::MenuItem("Toggle Demo")) {
				m_demoWindow = !m_demoWindow;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

	}

	if (m_demoWindow)
	ImGui::ShowDemoWindow();

	m_sceneHierachyPanel.update();
	m_inspecorWindow.update();
	m_viewportWindow.update();

}