#pragma once

#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include <memory>

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {

template <typename QueryPtr, typename Callback>
void ExecuteAsync(QueryPtr query, Callback&& callback) {
    boost::asio::co_spawn(query->get_conn_pool()->get_executor(), 
        [query, cb = std::forward<Callback>(callback)]() mutable -> boost::asio::awaitable<void> {
        try {
            auto conn = co_await query->get_conn_pool()->async_get_connection(boost::asio::use_awaitable);
            auto result = co_await query->Execute(conn); 
            cb(result); 
        } catch (const std::exception& e) {
            query->OnError(e);
        }
    }, 
        boost::asio::detached
    );
}

template <typename ExecuteLogic, typename OnError>
void ExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    ExecuteLogic&& execute_logic,
    OnError&& on_error
) {
    boost::asio::co_spawn(
        conn_pool->get_executor(),
        [conn_pool, 
        execute_logic = std::forward<ExecuteLogic>(execute_logic),
        on_error = std::forward<OnError>(on_error)
        ]() mutable -> boost::asio::awaitable<void> {
        try {
            auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);
            co_await execute_logic(conn);
        } catch (const std::exception& e) {
            std::move(on_error)(e);
        }
    },
        boost::asio::detached
    );
}

} // namespace dev::database
