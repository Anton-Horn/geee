#include "asset.h"
#include "rendering/stb_image.h"

namespace ec {

	void loadTexture2DAssetCPU(const Asset& asset, void*& assetDataPtr) {

		EC_ASSERT(asset.info.type == AssetType::TEXTURE_2D);
		EC_ASSERT(asset.state == AssetState::UNLOADED);

		TextureAssetData* assetData = new TextureAssetData();
			
		assetData->data = stbi_load(asset.assetPath.string().c_str(), (int*)&assetData->width, (int*)&assetData->height, (int*)&assetData->channels, 4);

		if (!assetData->data) EC_ERROR("Failed to load texture data!");

		assetDataPtr = assetData;
	
	}

	void unloadTexture2DAssetCPU(const Asset& asset, void*& assetDataPtr) {

		EC_ASSERT(asset.info.type == AssetType::TEXTURE_2D);
		EC_ASSERT(asset.state == AssetState::LOADED_CPU || asset.state == AssetState::LOADED_CPU_GPU);

		TextureAssetData* assetData = (TextureAssetData*)asset.assetData;

		stbi_image_free(assetData->data);

		delete assetData;

		
	}

}