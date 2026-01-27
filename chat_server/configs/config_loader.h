#pragma once

#include <boost/core/noncopyable.hpp>

namespace dev::chat_server {

class ConfigLoader : boost::noncopyable {
public:
    static bool LoadServerSetting(std::string_view path);
};

} // namespace dev::chat_server
