#pragma once

#include "utility/iguana/json_reader.hpp"

namespace dev::chat_server {

struct ServerInfo {
    boost::asio::ip::port_type port = 5000;
    int32_t processor_multiplier = 1;
};
REFLECTION(ServerInfo, port, processor_multiplier)

struct LogInfo {
    int32_t console_sink_level = 0;
    int32_t file_sink_level = 0;
    int32_t max_file_count = 0;
};
REFLECTION(LogInfo, console_sink_level, file_sink_level, max_file_count)

struct SqlInfo {
    std::string username;
    std::string password;
    std::string database;
    std::string host;
    uint16_t port = 0;
};
REFLECTION(SqlInfo, username, password, database, host, port)

struct ServerSetting {
    ServerInfo server_info;
    LogInfo log_info;
    SqlInfo sql_info;
};
REFLECTION(ServerSetting, server_info, log_info, sql_info)

class ServerConfig {
public:
    static bool Load(const std::string& setting_json);
    static const ServerSetting& server_setting();
};

} // namespace dev::chat_server
