#include "pch.h"
#include "chat_server/configs/server_setting.h"

#include <spdlog/spdlog.h>
#include <utility/iguana/json_reader.hpp>

#include "database/config/sql_info.h"

namespace dev::database {
REFLECTION(SqlInfo, username, password, database, host, port, thread_pool_count)
} //namespace dev::database

namespace dev::chat_server {

REFLECTION(ServerInfo, port, processor_multiplier)
REFLECTION(LogInfo, console_sink_level, file_sink_level, max_file_count)

REFLECTION(ServerSetting, server_info, log_info, sql_info)

static ServerSetting setting;

bool ServerConfig::Load(const std::string& setting_json) {
    std::error_code ec;

    iguana::from_json(setting, setting_json, ec);
    if (ec) {
        SPDLOG_ERROR("[ServerConfig] Unable to load. error message: {}", ec.message());
        return false;
    }
    return true;
}

const ServerSetting& ServerConfig::server_setting() {
    return setting;
}

} // namespace dev::chat_server
