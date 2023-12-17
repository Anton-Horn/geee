#include <imgui.h>

#include "editor/editor.h"


template<typename T>
void drawComponentUIImpl(T& component) {

}

template<> 
void drawComponentUIImpl<ec::TransformComponent>(ec::TransformComponent& component) {

	ImGui::Text("Transform");

	bool change = false;

	change |= ImGui::DragFloat3("Position", (float*)&component.positon);
	change |= ImGui::DragFloat3("Scale", (float*)&component.scale);
	change |= ImGui::DragFloat3("Rotation", (float*)&component.rotation);

	if (change) component.calculateTransform();

}

template<>
void drawComponentUIImpl<ec::QuadRenderComponent>(ec::QuadRenderComponent& component) {

	ImGui::Text("Quad");

	ImGui::ColorPicker4("Color", (float*) &component.color);


}

template<typename T>
void drawComponentUI(ec::Entity& e) {

	if (e.hasComponents<T>()) drawComponentUIImpl<T>(e.getComponent<T>());

}


void InspectorWindow::create(SharedWindowData* sharedData) {
	m_sharedData = sharedData;
}
void InspectorWindow::update() {

	ImGui::Begin("Inspector");

	if (!m_sharedData->selectedEntity.valid()) { ImGui::End(); return; }

	ImGui::Text(m_sharedData->selectedEntity.getTag().c_str());

	drawComponentUI<ec::TransformComponent>(m_sharedData->selectedEntity);
	
	drawComponentUI<ec::QuadRenderComponent>(m_sharedData->selectedEntity);

	ImGui::End();

}

void InspectorWindow::destroy() {

}