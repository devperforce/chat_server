#include <boost/asio.hpp>
#include <boost/mysql.hpp>

namespace dev::database {
class QueryAsyncBase;

template <typename ExecuteLogic, typename OnError>
void ExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    ExecuteLogic&& execute_logic, // (conn) -> awaitable<void> 형태의 함수
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

void ExecuteAsync2(std::shared_ptr<QueryAsyncBase> query);

} // namespace dev::database
