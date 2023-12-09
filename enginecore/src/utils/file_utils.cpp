#include "file_utils.h"

std::vector<uint8_t> ecUtilsReadBinaryFile(const std::filesystem::path& filePath) {

	std::streampos fileSize;
	std::ifstream file(filePath, std::ios::binary);
	EC_ASSERT(file.good());


	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);


	std::vector<uint8_t> fileData;
	fileData.resize(fileSize);
	file.read((char*)&fileData[0], fileSize);
	return fileData;

}


std::vector<uint32_t> ecUtilsReadBinaryFile32(const std::filesystem::path& filePath) {

	std::streampos fileSize;
	std::ifstream file(filePath, std::ios::binary);
	EC_ASSERT(file.good());


	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);


	std::vector<uint32_t> fileData;
	fileData.resize(fileSize / 4);
	file.read((char*)&fileData[0], fileSize);
	return fileData;

}

