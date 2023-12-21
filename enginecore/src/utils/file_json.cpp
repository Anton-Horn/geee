#include <fstream>

#include "file_json.h"

namespace ec {

	nlohmann::json jsonLoad(const std::filesystem::path& filePath) 
	{
		nlohmann::json result;
		std::ifstream in(filePath);
		in >> result;
		return result;
	}

	void jsonWrite(const std::filesystem::path& filePath, nlohmann::json& json)
	{

		std::ofstream out(filePath);
		out << std::setw(4) << json << std::endl;

	}

}