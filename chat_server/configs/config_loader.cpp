#include "pch.h"
#include "chat_server/configs/config_loader.h"

#include "chat_server/configs/server_setting.h"
#include "utility/file/file_loader.h"

namespace dev::chat_server {

bool ConfigLoader::LoadServerSetting(std::string_view path) {
    try {
        const std::string server_setting_json = utility::FileLoader(path).ReadFileToString();
        
        if (server_setting_json.empty()) {
            SPDLOG_WARN("Config file at '{}' is empty", path);
            return false;
        }
        
        if (!ServerConfig::Load(server_setting_json)) {
            SPDLOG_ERROR("Failed to load server configuration from '{}'", path);
            return false;
        }

        return true;
    } catch (const std::runtime_error& e) {
        SPDLOG_ERROR("Failed to load config from '{}': {}", path, e.what());
        return false;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Unexpected error loading config from '{}': {}", path, e.what());
        return false;
    }
}

} // namespace dev::chat_server
