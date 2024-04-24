#include <imgui.h>
#include <imgui_internal.h>

#include "editor/editor.h"
#include "scene/components.h"
#include "editor/ui/ui_widgets.h"

void SceneHierachyWindow::create(SharedWindowData* sharedData) {

	m_sharedData = sharedData;

}

uint32_t SceneHierachyWindow::getTreeNodeFlags(ec::Entity& entity)
{
	if (m_sharedData->selectedEntity == entity) return ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Selected;

	return ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
}

void SceneHierachyWindow::update() {


	bool open = ImGui::Begin("Scene Hierachy");

	if (!open) { ImGui::End(); return; }

	std::vector<ec::Entity> entities = m_sharedData->scene.getEntities();

	for (uint32_t i = 0; i < entities.size(); i++) {
		
		ec::Entity& e = entities[i];

		if (!e.valid()) continue;

		ec::IDComponent id = e.getComponent<ec::IDComponent>();
	
		bool open = ImGui::TreeNodeEx((void*)id.id, getTreeNodeFlags(e), id.tag.c_str());

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {

			m_sharedData->selectedEntity = e;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {

			ImGui::OpenPopup("EntityPopup");
		}

		if (ImGui::BeginPopup("EntityPopup")) {

			if (ImGui::Selectable("Rename Entity", false, ImGuiSelectableFlags_DontClosePopups)) {
				ImGui::OpenPopup("RenameEntityPopup");
			}

			if (ImGui::BeginPopup("RenameEntityPopup")) {

				uiWidgetsInputText_256(e.getTag(), "##Tag");

				ImGui::EndPopup();
			}

			if (ImGui::BeginMenu("Add Component")) {

				if (!e.hasComponents<ec::TransformComponent>() && ImGui::Selectable("Transform")) {			
					e.addComponent<ec::TransformComponent>();
				}

				if (!e.hasComponents<ec::QuadRenderComponent>() && ImGui::Selectable("Quad")) {
					e.addComponent<ec::QuadRenderComponent>(glm::vec4(1.0f));
				}

				ImGui::EndMenu();
			}

			

			if (ImGui::Selectable("Remove Entity")) {

				m_sharedData->scene.destroyEntity(e);
				m_sharedData->selectedEntity = ec::NullEntity{};

			}

			ImGui::EndPopup();
		}

			
		ImGui::TreePop();	

	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
		m_sharedData->selectedEntity = ec::NullEntity{};
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
		ImGui::OpenPopup("AddEntityPopup");
	}

	if (ImGui::BeginPopup("AddEntityPopup")) {

		if (ImGui::Selectable("Add Entity")) {

			m_sharedData->selectedEntity = m_sharedData->scene.createEntity();

		}

		ImGui::EndPopup();
	}

	ImGui::End();

}

void SceneHierachyWindow::destroy() {



}
