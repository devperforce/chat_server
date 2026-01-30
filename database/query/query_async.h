#pragma once

#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include <type_traits>
#include <concepts>
#include "utility/log.h"

namespace dev::database {

template <typename F, typename RowType>
concept ResultCallback = requires(F f, boost::system::error_code ec, boost::mysql::static_results<RowType> res) {
    {
        f(ec, std::move(res))
    } -> std::same_as<void>;
};

class QueryAsyncBase : boost::noncopyable {
public:
    explicit QueryAsyncBase(const IQueryContext& query_context)
        : query_context_(query_context) {
    }

protected:
    template <typename... Args, typename Callback>
    void ExecuteAsyncInternal(boost::mysql::with_params_t<Args...> bound_params, Callback&& callback) {
        auto conn_pool = query_context_.connection_pool();
        // Callback 시그니처에서 RowType 추출
        using RowType = typename callback_traits<std::decay_t<Callback>>::row_type;

        // 타입 안정성 체크
        static_assert(ResultCallback<Callback, RowType>, "Callback must match void(error_code, static_results<T>)");

        auto executor = conn_pool->get_executor();

        boost::asio::co_spawn(executor,
            [&logger = query_context_.logger(), conn_pool, params = std::move(bound_params), result_callback = std::forward<Callback>(callback)]() mutable
            -> boost::asio::awaitable<void> {

            boost::mysql::static_results<RowType> results;
#ifndef NDEBUG
            boost::mysql::diagnostics diag;
#endif

            try {
                auto conn = co_await conn_pool->async_get_connection(boost::asio::use_awaitable);
#ifndef NDEBUG
                co_await conn->async_execute(params, results, diag, boost::asio::use_awaitable);
#else
                co_await conn->async_execute(params, results, boost::asio::use_awaitable);
#endif
                result_callback(boost::system::error_code(), std::move(results));
            } catch (const boost::system::system_error& e) {
                logger.LogError(std::format(
#ifndef NDEBUG
                    "[ExecuteAsyncInternal] Async_execute failed. ec={}, what={}, server_message={}",
                    e.code().message(),
                    e.what(),
                    diag.server_message()
#else
                    "[ExecuteAsyncInternal] Async_execute failed. ec={}, what={}",
                    e.code().message(),
                    e.what()
#endif
                ));

                result_callback(e.code(), std::move(results));
            }
            catch (...) {
                result_callback(boost::asio::error::fault, std::move(results));
            }
        },
            boost::asio::detached
        );
    }

    const IQueryContext& query_context_;

private:
    template <typename T>
    struct callback_traits : callback_traits<decltype(&T::operator())> {
    };

    // 람다 및 함수 객체용 operator() 분석
    template <typename ClassType, typename ReturnType, typename Arg1, typename RowTypeArg>
    struct callback_traits<ReturnType(ClassType::*)(Arg1, boost::mysql::static_results<RowTypeArg>) const> {
        using row_type = RowTypeArg;
    };

    // 일반 함수 포인터
    template <typename ReturnType, typename Arg1, typename RowTypeArg>
    struct callback_traits<ReturnType(*)(Arg1, boost::mysql::static_results<RowTypeArg>)> {
        using row_type = RowTypeArg;
    };
};

} // namespace dev::database
