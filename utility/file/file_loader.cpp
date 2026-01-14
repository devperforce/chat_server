#include "pch.h"
#include "utility/file/file_loader.h"

#include <fstream>

namespace dev::utility {

FileLoader::FileLoader(const std::string& path) : path_(path) {

}

std::string FileLoader::ReadFileToString() const {
    std::ifstream input_file(path_);
    if (!input_file.is_open()) {
        throw std::runtime_error(std::format("Unable to open file! {}", path_));
    }
    return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

} // namespace dev::utility
