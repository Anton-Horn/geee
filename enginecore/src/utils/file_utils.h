#include <tiny_gltf.h>
#include "core/core.h"


std::vector<uint8_t> ecUtilsReadBinaryFile(const std::filesystem::path& filePath);

tinygltf::Model loadGltfModel(const std::filesystem::path& filepath);