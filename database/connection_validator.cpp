#include "pch.h"
#include "database/connection_validator.h"

#include "utility/log/logger.h"

namespace dev::database {
using namespace boost;

static asio::awaitable<bool> TestQuery(
    const utility::ILogger& logger,
    mysql::connection_pool& db_conn_pool
) {
    try {
        auto conn = co_await db_conn_pool.async_get_connection(asio::use_awaitable);

        mysql::results res;
        co_await conn->async_execute("SELECT 1", res, asio::use_awaitable);

        co_return true; // 성공
    } catch (const std::exception& e) {
        logger.LogError("[ConnectionValidator] Exception occurred. error: {}", e.what());
        co_return false; // 실패
    }
}

ConnectionValidator::ConnectionValidator(
    const utility::ILogger& logger
) : logger_(logger) {

}

bool ConnectionValidator::Check(
    boost::mysql::connection_pool& db_conn_pool,
    std::chrono::steady_clock::duration wait_time
) const {
    try {
        std::future<bool> conn_future = asio::co_spawn(
            db_conn_pool.get_executor(), 
            TestQuery(logger_, db_conn_pool), 
            asio::use_future
        );

        auto db_conn_status = conn_future.wait_for(wait_time);
        if (db_conn_status == std::future_status::timeout) {
            logger_.LogError("[ConnectionValidator] Timeout occurred.");
            return false;
        }

        if (!conn_future.get()) {
            logger_.LogError("[ConnectionValidator] Fail to get future.");
            return false;
        }

        logger_.LogInfo("[ConnectionValidator] Successfully connected to database.");
        return true;
    } catch (const std::exception& e) {
        logger_.LogError("[ConnectionValidator] Exception occurred. error: {}", e.what());
        return false;
    }
}

} // namespace dev::database
