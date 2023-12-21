#include "asset/asset.h"
#include "core/random.h"

namespace ec {
	void AssetManager::create(JobSystem& jobSystem)
	{
		m_jobSystem = &jobSystem;
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
		
		uint64_t id = randomInt<uint64_t>();
		
		{
			std::lock_guard<std::mutex> assetsLock(m_mutex);
			m_assets.try_emplace(id);
			Asset& asset = m_assets.at(id);
			asset.extension = getExtensionByPath(assetPath);
			asset.type = getTypeByExtension(asset.extension);
			asset.state = AssetState::UNLOADED;
			asset.assetPath = assetPath;
		}

		return { id };
	}

	const Asset& AssetManager::getAsset(const AssetHandle& handle)
	{
		
		if (!exists(handle)) { EC_ERROR("Asset cant be accessed!"); return; }

		return m_assets.at(handle.assetID);
	}

	void AssetManager::loadAssetCPU(const AssetHandle& handle)
	{
		if (!exists(handle)) { EC_ERROR("Asset cant be accessed!"); return; }

		std::lock_guard<std::mutex> lock(m_mutex);

		Asset& asset = m_assets.at(handle.assetID);

		switch (asset.type) {
		case AssetType::TEXTURE_2D:
			m_jobSystem->queueJob([&asset] {
				loadTexture2DAssetCPU(asset);
			});
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

	bool AssetManager::exists(const AssetHandle& handle)
	{
		return m_assets.find(handle.assetID) != m_assets.end();
	}


}