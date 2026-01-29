#pragma once

#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include <memory>
#include "utility/log.h"

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {


/*
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
*/

template <typename ExecuteLogic, typename OnError>
void ExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    ExecuteLogic&& execute_logic,
    OnError&& on_error
) {
    //using ReturnType = std::invoke_result_t<ExecuteLogic, boost::mysql::pooled_connection>;
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

// 콜백의 인자 타입을 분석하여 RowType을 찾아내는 헬퍼
template <typename T>
struct extract_row_type;

// 콜백이 void(error_code, static_results<RowType>) 형태일 때 RowType 추출
template <typename RowType>
struct extract_row_type<std::function<void(boost::system::error_code, boost::mysql::static_results<RowType>)>> {
    using type = RowType;
};

template <typename... Args, typename Callback>
void ExecuteAsync(
    std::shared_ptr<boost::mysql::connection_pool> conn_pool,
    boost::mysql::with_params_t<Args...> bound_params,
    Callback&& callback
) {
    // 람다 함수의 시그니처로부터 RowType을 추론 (traits 활용)
    // 인자로부터 직접 추론하기 위해 도우미 객체를 생성하거나 타입을 강제합니다.
    using CallbackType = decltype(std::function(std::forward<Callback>(callback)));
    using RowType = typename extract_row_type<CallbackType>::type;

    auto executor = conn_pool->get_executor();

    auto task = [
        conn_pool, 
        bound_params = std::move(bound_params), 
        callback = std::forward<Callback>(callback)
    ]() -> boost::asio::awaitable<void> {
        boost::mysql::static_results<RowType> results;
        boost::mysql::diagnostics diag; // diagnostics 객체를 미리 선언
        try {
            auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);
            co_await conn->async_execute(bound_params, results, diag, boost::asio::use_awaitable);

            callback(boost::system::error_code(), std::move(results));
        } catch (const boost::system::system_error& e) {
            // 여기서 e.code()는 MySQL 에러 코드를 가지고 있습니다.
            // 상세 서버 메시지가 필요하다면 diag.server_message()를 사용하세요.
            LOG_ERROR("SQL Error: {}, Server Msg: {}", e.code().message(), diag.server_message());
            callback(e.code(), std::move(results));
        } catch (const std::exception& e) {
            LOG_ERROR("Unknown exception: {}", e.what());
            callback(make_error_code(boost::system::errc::io_error), std::move(results));
        }
    };

    boost::asio::co_spawn(executor, std::move(task), boost::asio::detached);
}

} // namespace dev::database
