#pragma once

#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <boost/core/noncopyable.hpp>

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {

template <typename Result>
class QueryAsyncBase : boost::noncopyable {
public:
    using ResultType = std::shared_ptr<Result>; // 결과 타입 정의

    QueryAsyncBase(
        const utility::ILogger& logger,
        std::shared_ptr<boost::mysql::connection_pool> conn_pool
    ) : logger_(logger), conn_pool_(conn_pool) {
    }

    virtual ~QueryAsyncBase() {
    }

    virtual boost::asio::awaitable<ResultType> Execute(boost::mysql::pooled_connection& conn) = 0;
    virtual void OnError(const std::exception& e) const = 0;

    std::shared_ptr<boost::mysql::connection_pool> get_conn_pool() const {
        return conn_pool_;
    }

protected:
    const utility::ILogger& logger_;
    std::shared_ptr<boost::mysql::connection_pool> conn_pool_;
};

template <typename QueryPtr, typename Callback>
void ExecuteAsync(QueryPtr query, Callback&& callback) {
    boost::asio::co_spawn(query->get_conn_pool()->get_executor(), 
        [query, cb = std::forward<Callback>(callback)]() mutable -> boost::asio::awaitable<void> {
        try {
            auto conn = co_await query->get_conn_pool()->async_get_connection(boost::asio::use_awaitable);

            // Execute의 반환값을 받습니다.
            auto result = co_await query->Execute(conn); 

            // 콜백을 호출합니다.
            cb(result); 

        } catch (const std::exception& e) {
            query->OnError(e);
        }
    }, 
        boost::asio::detached
    );
}


} // namespace dev::database
