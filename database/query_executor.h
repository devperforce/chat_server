#include <boost/asio.hpp>
#include <boost/mysql.hpp>

namespace dev::database {

template <typename ResultType, typename QueryFunc, typename OnSuccess, typename OnError>
void ExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    QueryFunc&& query_factory,
    OnSuccess&& on_success,
    OnError&& on_error
) {
    boost::asio::co_spawn(
        conn_pool->get_executor(),
        [conn_pool, 
        query_factory = std::forward<QueryFunc>(query_factory),
        on_success    = std::forward<OnSuccess>(on_success), 
        on_error      = std::forward<OnError>(on_error)
        ]() mutable -> boost::asio::awaitable<void> {
        try {
            // 커넥션 획득
            auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);

            // 결과셋 준비
            boost::mysql::static_results<ResultType> results;

            // 쿼리 실행
            co_await conn->async_execute(query_factory(), results, boost::asio::use_awaitable);

            std::move(on_success)(results.rows());
        } catch (const std::exception& e) {
            std::move(on_error)(e);
        }
    },
        boost::asio::detached
    );
}

template <typename QueryFunc, typename OnError>
void JustExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    QueryFunc&& query_factory,
    OnError&& on_error
) {
    boost::asio::co_spawn(
        conn_pool->get_executor(),
        [conn_pool, 
        query_factory = std::forward<QueryFunc>(query_factory),
        on_error      = std::forward<OnError>(on_error)
        ]() mutable -> boost::asio::awaitable<void> {
        try {
            auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);

            boost::mysql::results result; 

            co_await conn->async_execute(
                query_factory(), 
                result,
                boost::asio::use_awaitable
            );

        } catch (const std::exception& e) {
            std::move(on_error)(e);
        }
    },
        boost::asio::detached
    );
}

} // namespace dev::database
