#include <imgui.h>

#include "editor/editor.h"


void AssetManagerWindow::create(SharedWindowData* sharedData) {
	m_sharedData = sharedData;
}

void AssetManagerWindow::update() {

	ImGui::Begin("Assets");

	for (ec::Asset& asset : m_assets) {

		ImGui::Text(std::to_string(asset.info.handle.assetID).c_str());
		ImGui::Text(asset.assetPath.string().c_str());
		ImGui::Text(ec::assetStateToString(asset.state).c_str());
		ImGui::Text(ec::assetTypeToString(asset.info.type).c_str());

		if (ImGui::Button("Load")) {

			ec::Application::getInstance().getAssetManager().loadAssetCPU(asset.info.handle);

		}

		ImGui::Separator();

	}
	if (ImGui::Button("Add Asset")) {

		ec::AssetHandle handle = ec::Application::getInstance().getAssetManager().createAsset("data/textures/image.png");
		

	}
	ImGui::End();

}

void AssetManagerWindow::synchronizedUpdate()
{

	m_assets = ec::Application::getInstance().getAssetManager().getAssetsCopy();

}

void AssetManagerWindow::destroy() {

}