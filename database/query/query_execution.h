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

template <typename ExecuteLogic, typename OnError>
void ExecuteAsync(
    const IQueryContext& query_context,
    ExecuteLogic&& execute_logic,
    OnError&& on_error
) {
    //using ReturnType = std::invoke_result_t<ExecuteLogic, boost::mysql::pooled_connection>;
    boost::asio::co_spawn(
        query_context.connection_pool()->get_executor(),
        [conn_pool =     query_context.connection_pool(), 
         execute_logic = std::forward<ExecuteLogic>(execute_logic),
         on_error =      std::forward<OnError>(on_error)
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

// 콜백이 올바른 시그니처인지 확인
// void(error_code, static_results<T>) 형태인지 검사
template <typename F, typename RowType>
concept MysqlStaticCallback = requires(F f, boost::system::error_code ec, boost::mysql::static_results<RowType> res) {
    { f(ec, std::move(res)) } -> std::same_as<void>;
};

// 콜백 인자에서 RowType을 추출하기 위한 Helper
// std::invoke_result는 반환값만 가져오므로, 인자 추출을 위해 traits를 사용
template <typename T>
struct callback_traits : callback_traits<decltype(&T::operator())> {};

// 멤버 함수 포인터(람다의 operator())를 위한 특수화
template <typename C, typename R, typename Arg1, typename RowTypeArg>
struct callback_traits<R(C::*)(Arg1, boost::mysql::static_results<RowTypeArg>) const> {
    using row_type = RowTypeArg;
};

// 일반 함수 포인터를 위한 특수화
template <typename R, typename Arg1, typename RowTypeArg>
struct callback_traits<R(*)(Arg1, boost::mysql::static_results<RowTypeArg>)> {
    using row_type = RowTypeArg;
};

template <typename... Args, typename Callback>
void ExecuteAsync(
    const IQueryContext& query_context,
    boost::mysql::with_params_t<Args...> bound_params,
    Callback&& callback
) {
    // RowType 추출 (std::decay_t로 레퍼런스 및 const 제거)
    using PureCallback = std::remove_cvref_t<Callback>;
    using RowType = typename callback_traits<PureCallback>::row_type;

    // Concept을 이용한 컴파일 타임 검증
    static_assert(MysqlStaticCallback<PureCallback, RowType>, 
        "Callback signature must be void(error_code, static_results<RowType>)");

    auto task = [
        &logger = query_context.logger(),
        conn_pool = query_context.connection_pool(), 
        params = std::move(bound_params), 
        cb = std::forward<Callback>(callback)
    ]() mutable -> boost::asio::awaitable<void> {
        boost::mysql::static_results<RowType> results;

#if defined(_DEBUG) || !defined(NDEBUG)
        boost::mysql::diagnostics diag;
#endif
        try {
            auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);

#if defined(_DEBUG) || !defined(NDEBUG)
            co_await conn->async_execute(params, results, diag, boost::asio::use_awaitable);
#else
            co_await conn->async_execute(params, results, boost::asio::use_awaitable);
#endif

            cb(boost::system::error_code(), std::move(results));
        } catch (const boost::system::system_error& e) {
#if defined(_DEBUG) || !defined(NDEBUG)
            logger.LogError(std::format(
                "[ExecuteAsync] Async_execute failed. ec={}, what={}, server_message={}",
                e.code().message(),
                e.what(),
                diag.server_message()
            ));
#else
            logger.LogError(std::format(
                "[ExecuteAsync] Async_execute failed. ec={}, what={}",
                e.code().message(),
                e.what()
            ));
#endif
            cb(e.code(), std::move(results));
        } catch (const std::exception&) {
            cb(boost::system::errc::make_error_code(boost::system::errc::io_error), std::move(results));
        }
    };

    boost::asio::co_spawn(query_context.connection_pool()->get_executor(), std::move(task), boost::asio::detached);
}


template <typename _RowType, typename... _Args>
boost::asio::awaitable<boost::mysql::static_results<_RowType>> ExecuteAsync(
    const IQueryContext& query_context,
    boost::mysql::with_params_t<_Args...> bound_params
) {
    auto& logger = query_context.logger();
    auto conn_pool = query_context.connection_pool();
    boost::mysql::static_results<_RowType> results;

#if defined(_DEBUG) || !defined(NDEBUG)
    boost::mysql::diagnostics diag;
#endif

    try {
        auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);

#if defined(_DEBUG) || !defined(NDEBUG)
        co_await conn->async_execute(bound_params, results, diag, boost::asio::use_awaitable);
#else
        co_await conn->async_execute(bound_params, results, boost::asio::use_awaitable);
#endif
        co_return results;

    } catch (const boost::system::system_error& e) {
        throw e;
    }
}

} // namespace dev::database
