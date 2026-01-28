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

class QueryAsyncBase : boost::noncopyable {
public:
    QueryAsyncBase(const utility::ILogger& logger, std::shared_ptr<boost::mysql::connection_pool> conn_pool);
    virtual ~QueryAsyncBase();

    virtual boost::asio::awaitable<void> Execute(boost::mysql::pooled_connection& conn) = 0;
    virtual void OnError(const std::exception& e) const = 0;

    std::shared_ptr<boost::mysql::connection_pool> get_conn_pool() const;

protected:
    const utility::ILogger& logger_;
    std::shared_ptr<boost::mysql::connection_pool> conn_pool_;
};

} // namespace dev::database
