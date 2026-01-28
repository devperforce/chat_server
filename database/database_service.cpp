#include "pch.h"
#include "database/database_service.h"

#include "utility/log/logger.h"
#include "database/config/sql_info.h"

namespace dev::database {

static std::shared_ptr<boost::mysql::connection_pool> CreateDBConnectionPool(
    boost::asio::thread_pool& db_thread_pool,
    const SqlInfo& sql_info
) {
    boost::mysql::pool_params params{
        .username = sql_info.username,
        .password = sql_info.password,
        .database = sql_info.database,
        .thread_safe = true
    };
    params.server_address.emplace_host_and_port(sql_info.host, sql_info.port);
    auto db_connection_pool = std::make_shared<boost::mysql::connection_pool>(db_thread_pool, std::move(params));
    BOOST_ASSERT(db_connection_pool != nullptr);
    return db_connection_pool;
}

DatabaseService::DatabaseService(
    const utility::ILogger& logger,
    boost::asio::thread_pool& thread_pool,
    const SqlInfo& sql_info
) : logger_(logger), thread_pool_(thread_pool), sql_info_(sql_info) {
}

DatabaseService::~DatabaseService() {
    logger_.LogInfo("~DatabaseService");
}

bool DatabaseService::Start() {
    connection_pool_ = CreateDBConnectionPool(thread_pool_, sql_info_);
    connection_pool_->async_run(boost::asio::detached);


    return true;
}

void DatabaseService::Stop() {
    thread_pool_.stop();
    logger_.LogInfo("[ChatServer] Waiting for remaining DB tasks...");
    thread_pool_.join();
    logger_.LogInfo("[ChatServer] Database thread pool stopped safely.");
}

std::shared_ptr<boost::mysql::connection_pool> DatabaseService::connection_pool() const {
    BOOST_ASSERT(connection_pool_ != nullptr);
    return connection_pool_;
}

} // namespace dev::database
