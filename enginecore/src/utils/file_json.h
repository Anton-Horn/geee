#pragma once
#include <nlohmann/json.hpp>

#include "core/core.h"

namespace ec {

	nlohmann::json jsonLoad(const std::filesystem::path& filePath);
	void jsonWrite(const std::filesystem::path& filePath, nlohmann::json& json);

}