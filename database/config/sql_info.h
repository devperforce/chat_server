#pragma once

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::database {

struct SqlInfo {
    std::string username;
    std::string password;
    std::string database;
    std::string host;
    uint16_t port = 0;
    int32_t thread_pool_count = 4;
};

} // namespace dev::database
