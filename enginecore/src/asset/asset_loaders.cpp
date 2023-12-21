#include "asset.h"
#include "rendering/stb_image.h"

namespace ec {

	void loadTexture2DAsset(Asset& asset) {

		EC_ASSERT(asset.type == AssetType::TEXTURE_2D);

		std::unique_ptr<TextureAssetData> assetData = std::make_unique<TextureAssetData>();	
		assetData->data = stbi_load(asset.assetPath.string().c_str(), (int*)&assetData->width, (int*)&assetData->height, (int*)&assetData->channels, 4);
		asset.assetData = std::move(assetData);
	}

	void unloadTextureAsset(Asset& asset) {

		EC_ASSERT(asset.type == AssetType::TEXTURE_2D);
		
		auto* ptr = asset.assetData.get();
		asset.assetData.release();
		delete ptr;
		
	}

}