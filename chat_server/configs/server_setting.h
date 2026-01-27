#pragma once

#include <string>
#include <cstdint>
#include <boost/asio.hpp>

#include "database/config/sql_info.h"

namespace dev::chat_server {

struct ServerInfo {
    boost::asio::ip::port_type port = 5000;
    int32_t processor_multiplier = 1;
};

struct LogInfo {
    int32_t console_sink_level = 0;
    int32_t file_sink_level = 0;
    int32_t max_file_count = 0;
};

struct ServerSetting {
    ServerInfo server_info;
    LogInfo log_info;
    database::SqlInfo sql_info; // 외부 의존성
};

class ServerConfig {
public:
    static bool Load(const std::string& setting_json);
    static const ServerSetting& server_setting();
};

} // namespace dev::chat_server
