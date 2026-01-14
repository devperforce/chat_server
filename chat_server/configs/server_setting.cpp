#include "pch.h"
#include "chat_server/configs/server_setting.h"

#include <spdlog/spdlog.h>
#include <utility/iguana/json_reader.hpp>

#include "system/singleton.h"

namespace dev::chat_server {

class ServerConfigRepository : public system::Singleton<ServerConfigRepository> {
public:
    ServerSetting server_setting;
};

bool ServerConfig::Load(const std::string& setting_json) {
    std::error_code ec;

    ServerSetting server_setting;
    iguana::from_json(server_setting, setting_json, ec);
    if (ec) {
        SPDLOG_ERROR("[ServerConfig] Unable to load. error message: {}", ec.message());
        return false;
    }
    ServerConfigRepository::GetInstance().server_setting = std::move(server_setting);
    return true;
}

const ServerSetting& ServerConfig::server_setting() {
    return ServerConfigRepository::GetInstance().server_setting;
}

} // namespace dev::chat_server
