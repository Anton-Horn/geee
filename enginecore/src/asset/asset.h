#pragma once
#include <filesystem>
#include <unordered_map>
#include <thread>
#include <mutex>

#include "core/core.h"
#include "core/job_system.h"

namespace ec {

	enum class AssetType {

		UNKNOWN,
		TEXTURE_2D,
		MODEL

	};

	std::string assetTypeToString(AssetType type);

	enum class AssetExtension {

		UNKNOWN,
		JPG,
		PNG,
		GLTF

	};

	enum class AssetState {

		UNLOADED,
		LOADED_CPU,
		LOADED_GPU,
		LOADED_CPU_GPU

	};

	std::string assetStateToString(AssetState state);

	enum class AssetManagerLoadingType {

		BASIC_FILE,
		ASSET_PACK

	};

	struct AssetHandle {

		uint64_t assetID;

	};

	struct Asset {

		//relative asset path from working dir, if AssetManagerLoadingType = ASSET_PACK, path should be empty
		
		void* assetData;
		std::filesystem::path assetPath;
		AssetState state;
		
		struct AssetInfo {

			AssetHandle handle;
			AssetType type;
			AssetExtension extension;

		} const info;

		Asset() : info({0, AssetType::UNKNOWN, AssetExtension::UNKNOWN}), assetData(nullptr), assetPath(), state(AssetState::UNLOADED) {}

		Asset(void* assetData, const std::filesystem::path& assetPath, AssetState state, const AssetInfo& info) : 
			assetData(assetData), assetPath(assetPath),state(state), info(info)  {}

	};

	struct TextureAssetData {

		uint8_t* data = nullptr;

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t channels = 0;

	};

	// Asset Loaders
	void loadTexture2DAssetCPU(const Asset& asset, void*& assetDataPtr);
	void unloadTexture2DAssetCPU(const Asset& asset, void*& assetDataPtr);

	class AssetManager {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(AssetManager)

		void create(JobSystem& jobSystem);
		void destroy();

		void loadAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType);
		void saveAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType);

		//Thread safe
		AssetHandle createAsset(const std::filesystem::path& assetPath);

		const void* getAssetData(const AssetHandle& handle);
		AssetState getAssetState(const AssetHandle& handle);

		void loadAssetCPU(const AssetHandle& handle);
		void loadAssetGPU(const AssetHandle& handle);

		void unloadAssetCPU(const AssetHandle& handle);
		void unloadAssetGPU(const AssetHandle& handle);

		bool exists(const AssetHandle& handle);
		bool exists(const Asset& asset);

		void assetChangeState(const AssetHandle& handle, AssetState state);

		//should only be called from main thread while no other thread is accessing the asset manager
		void handleStates();

		// returns copy of all assets in undefined order
		std::vector<Asset> getAssetsCopy();

	private:

		AssetType getTypeByExtension(AssetExtension ex);
		AssetExtension getExtensionByPath(const std::filesystem::path& path);

		std::mutex m_mutex;

		std::mutex m_stateMutex;

		std::unordered_map<uint64_t, Asset> m_assets;

		AssetManagerLoadingType m_loadingType;
		std::filesystem::path m_assetPackPath;

		std::vector<std::tuple<AssetState, AssetHandle>> m_assetStateChanges;

		JobSystem* m_jobSystem;

	};

}