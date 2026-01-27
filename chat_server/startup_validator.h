#pragma once

#include <string>
#include <boost/core/noncopyable.hpp>

namespace dev::chat_server {
    struct ServerSetting;
} // namespace dev::chat_server

namespace dev::database {
    class DatabaseService;
} // namespace dev::database

namespace dev::utility {
    class ILogger;
} // namespace dev::utility

namespace dev::chat_server {

class StartupValidator : boost::noncopyable {
public:
    explicit StartupValidator(
        utility::ILogger& logger,
        const ServerSetting& server_setting,
        database::DatabaseService& db_service
    );

    bool Validate() const;

private:
    utility::ILogger& logger_;
    const ServerSetting& server_setting_;
    database::DatabaseService& db_service_;
};

} // namespace dev::chat_server
