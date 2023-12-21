#pragma once
#include <filesystem>
#include <unordered_map>
#include <thread>

#include "core/core.h"
#include "core/job_system.h"

namespace ec {

	enum class AssetType {

		UNKNOWN,
		TEXTURE_2D,
		MODEL

	};

	enum class AssetExtension {

		UNKNOWN,
		JPG,
		PNG,
		GLTF

	};

	enum class AssetState {

		UNLOADED,
		PENDING_CPU,
		LOADED_CPU,
		PENDING_GPU,
		LOADED_GPU

	};

	enum class AssetManagerLoadingType {

		BASIC_FILE,
		ASSET_PACK

	};

	struct Asset {

		AssetType type;
		AssetExtension extension;
		AssetState state;

		//relative asset path from working dir, if AssetManagerLoadingType = ASSET_PACK, path should be empty
		std::filesystem::path assetPath;

		std::unique_ptr<void> assetData;
		std::unique_ptr<void> asset;

		std::mutex assetMutex;

		Asset() = default;

	};

	struct AssetHandle {

		uint64_t assetID;

	};

	struct TextureAssetData {

		uint8_t* data = nullptr;

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t channels = 0;

	};

	void loadTexture2DAssetCPU(Asset& asset);
	void unloadTexture2DAssetCPU(const Asset& asset);

	class AssetManager {

	public:

		EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(AssetManager)

		void create(JobSystem& jobSystem);
		void destroy();

		void loadAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType);
		void saveAssetsInfo(const std::filesystem::path& assetPath, AssetManagerLoadingType loadingType);

		//Thread safe
		AssetHandle createAsset(const std::filesystem::path& assetPath);

		const Asset& getAsset(const AssetHandle& handle);

		void loadAssetCPU(const AssetHandle& handle);
		void loadAssetGPU(const AssetHandle& handle);

		void unloadAssetCPU(const AssetHandle& handle);
		void unloadAssetGPU(const AssetHandle& handle);

		bool exists(const AssetHandle& handle);

	private:

		AssetType getTypeByExtension(AssetExtension ex);
		AssetExtension getExtensionByPath(const std::filesystem::path& path);

		std::mutex m_mutex;

		std::unordered_map<uint64_t, Asset> m_assets;

		AssetManagerLoadingType m_loadingType;
		std::filesystem::path m_assetPackPath;

		JobSystem* m_jobSystem;

	};

}