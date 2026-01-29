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
    using ResultType = std::shared_ptr<Result>;

    QueryAsyncBase(
        const IQueryContext& database_context
    ) : logger_(database_context.logger()), conn_pool_(database_context.connection_pool()) {
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

} // namespace dev::database
