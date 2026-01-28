#include "pch.h"
#include "startup_validator.h"

#include "database/connection_validator.h"
#include "utility/system_status.h"
#include "utility/log.h"
#include "chat_server/configs/server_setting.h"
#include "database/database_service.h"
#include "utility/log/logger.h"

namespace dev::chat_server {

static bool ValidatePort(
    const ServerSetting& server_setting
) {
    if (utility::PortInUse(server_setting.server_info.port)) {
        return false;
    }
    return true;;
}

bool StartupValidator::Validate() const {
    auto port_result = ValidatePort(server_setting_);
    if (!port_result) {
        logger_.LogError("[StartupValidator] Port already in use. port: {}", 
            server_setting_.server_info.port);
        return port_result;
    }

    logger_.LogInfo("[StartupValidator] Starting database connection validation...");

    auto db_result = database::ConnectionValidator(logger_).Check(
        *db_service_.connection_pool(),
        std::chrono::seconds(5)
    );
    if (!db_result) {
        logger_.LogError("[StartupValidator] Fail to connect database.");
        return db_result;
    }

    logger_.LogInfo("[StartupValidator] Database connection validation completed successfully.");
    return true;
}
    
StartupValidator::StartupValidator(
    utility::ILogger& logger,
    const ServerSetting& server_setting,
    database::DatabaseService& db_service)
    : logger_(logger),
      server_setting_(server_setting),
      db_service_(db_service) {
}

} // namespace dev::chat_server