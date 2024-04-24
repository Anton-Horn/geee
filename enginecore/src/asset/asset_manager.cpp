#include "asset/asset.h"
#include "core/random.h"

namespace ec {

	void AssetManager::create(JobSystem& jobSystem)
	{
		m_jobSystem = &jobSystem;
		m_assetStateChanges.reserve(100);
	}
	void AssetManager::destroy()
	{

	}

	void AssetManager::loadAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType)
	{



	}

	void AssetManager::saveAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType)
	{
	}

	AssetHandle ec::AssetManager::createAsset(const std::filesystem::path& assetPath)
	{
		
		//makes 0 an invalid handle
		uint64_t id = std::min(randomInt<uint64_t>() + 1, std::numeric_limits<uint64_t>::max());
	
		Asset asset(nullptr, assetPath, AssetState::UNLOADED, { id, getTypeByExtension(getExtensionByPath(assetPath)), getExtensionByPath(assetPath) });
			
		std::lock_guard<std::mutex> assetsLock(m_mutex);

		m_assets.try_emplace(id, nullptr, assetPath, AssetState::UNLOADED, Asset::AssetInfo{ id, getTypeByExtension(getExtensionByPath(assetPath)), getExtensionByPath(assetPath) });		

		return { id };
	}

	const void* AssetManager::getAssetData(const AssetHandle& handle)
	{
		
		if (!exists(handle)) { EC_ERROR("Asset cant be accessed!"); return {}; }

		const Asset& asset = m_assets.at(handle.assetID);
		if (asset.state == AssetState::UNLOADED) { EC_ERROR("Tried to access an unloaded asset!"); return {}; }
		return asset.assetData;

	}

	AssetState AssetManager::getAssetState(const AssetHandle& handle)
	{
		if (!exists(handle)) { EC_ERROR("Asset cant be accessed!"); return {}; }
		return m_assets.at(handle.assetID).state;
	}

	void AssetManager::loadAssetCPU(const AssetHandle& handle)
	{
		if (!exists(handle)) { EC_ERROR("Asset cant be accessed!"); return; }

		Asset& asset = m_assets.at(handle.assetID);

		if (asset.state != AssetState::UNLOADED) { EC_ERROR("tried to load an asset in an invalid state"); return; }

		switch (asset.info.type) {
		case AssetType::TEXTURE_2D:
			m_jobSystem->queueJob([&] {
				loadTexture2DAssetCPU(asset, asset.assetData);
				assetChangeState(asset.info.handle, AssetState::LOADED_CPU);
			},false);

			break;
		case AssetType::MODEL:
		case AssetType::UNKNOWN:
		default:
			EC_ERROR("Unknown or missing asset type while loading asset for the cpu");
		}

	}

	void AssetManager::loadAssetGPU(const AssetHandle& handle)
	{
	}

	void AssetManager::unloadAssetCPU(const AssetHandle& handle) {

	}
	void AssetManager::unloadAssetGPU(const AssetHandle& handle) {

	}

	AssetType AssetManager::getTypeByExtension(AssetExtension ex)
	{
		
		switch (ex) {

			case AssetExtension::JPG:
			case AssetExtension::PNG:
				return AssetType::TEXTURE_2D;
			case AssetExtension::GLTF:
				return AssetType::MODEL;

		}

		return AssetType::UNKNOWN;

	}

	AssetExtension AssetManager::getExtensionByPath(const std::filesystem::path& path)
	{
		std::string ex = path.extension().string();

		if (ex == ".jpg") return AssetExtension::JPG;
		if (ex == ".png") return AssetExtension::PNG;
		if (ex == ".gltf") return AssetExtension::GLTF;

		return AssetExtension::UNKNOWN;
	}
	  
	void AssetManager::handleStates()
	{

		if (m_assetStateChanges.empty()) return;

		for (auto& stateChange : m_assetStateChanges) {

			if (exists(std::get<1>(stateChange))) EC_ERROR("Tried to change state of a non existing asset");

			Asset& asset = m_assets.at(std::get<1>(stateChange).assetID);
			asset.state = std::get<0>(stateChange);

		}
		m_assetStateChanges.clear();
	}

	void AssetManager::assetChangeState(const AssetHandle& handle, AssetState state)
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
	
		m_assetStateChanges.push_back(std::make_tuple(state, handle));
	}

	bool AssetManager::exists(const AssetHandle& handle)
	{
		if (handle.assetID == 0) return false;
		return m_assets.find(handle.assetID) != m_assets.end();
	}

	bool AssetManager::exists(const Asset& asset)
	{
		return exists(asset.info.handle);
	}

	std::vector<Asset> AssetManager::getAssetsCopy() {
		std::vector<Asset> result;
		result.reserve(m_assets.size());

		for (auto& asset : m_assets) {

			result.push_back(asset.second);

		}

		return result;
	}

	std::string assetStateToString(AssetState state) {
		switch (state) {
		case AssetState::UNLOADED:
			return "UNLOADED";
		case AssetState::LOADED_CPU:
			return "LOADED_CPU";
		case AssetState::LOADED_GPU:
			return "LOADED_GPU";
		case AssetState::LOADED_CPU_GPU:
			return "LOADED_CPU_GPU";
		default:
			return "UNKNOWN_STATE";
		}
	}

	std::string assetTypeToString(AssetType type) {
		switch (type) {
		case AssetType::UNKNOWN:
			return "UNKNOWN";
		case AssetType::TEXTURE_2D:
			return "TEXTURE_2D";
		case AssetType::MODEL:
			return "MODEL";
		default:
			return "INVALID_TYPE";
		}
	}


}