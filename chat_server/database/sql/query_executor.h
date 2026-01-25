#include <iostream>
#include <string>
#include <vector>
#include <expected> // C++23 필요 (또는 <boost/outcome.hpp> 등으로 대체 가능)
#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include <boost/describe.hpp> // 구조체 매핑용

namespace mysql = boost::mysql;
namespace asio = boost::asio;

enum class ErrorMsg {
    kLogicalError,
    kNotAffected,
    kException
};

template <typename ResultType, typename QueryFunc, typename OnSuccess, typename OnError>
void EXECUTE_QUERY(
    mysql::connection_pool& pool,
    QueryFunc&& query_factory,
    OnSuccess&& on_success,
    OnError&& on_error
) {
    asio::co_spawn(
        pool.get_executor(),
        [
            &pool, 
            query_factory = std::forward<QueryFunc>(query_factory),
            on_success = std::forward<OnSuccess>(on_success),
            on_error = std::forward<OnError>(on_error)
        ]() -> asio::awaitable<void> {
        try {
            // 1. 커넥션 획득
            auto conn = co_await pool.async_get_connection(asio::use_awaitable);

            // 2. 결과셋 준비 (static_results 사용 시 DBUser와 매핑 필요)
            mysql::static_results<ResultType> results;

            // 3. 쿼리 실행
            co_await conn->async_execute(query_factory(), results, asio::use_awaitable);

            // 4. 성공 콜백 호출 (rows 반환)
            on_success(results.rows());

        } catch (const std::exception& e) {
            // 5. 예외 발생 시 에러 콜백 호출
            on_error(e);
        }
    },
        asio::detached
    );
}