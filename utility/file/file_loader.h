#pragma once

#include <boost/core/noncopyable.hpp>

namespace dev::utility {

class FileLoader : boost::noncopyable {
public:
    explicit FileLoader(std::string_view path);

    std::string ReadFileToString() const;

private:
    const std::string path_;
};

} // namesapce dev::utility
