#pragma once

#include <boost/core/noncopyable.hpp>

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {

struct SqlInfo;

class DatabaseService : boost::noncopyable {
public:
    DatabaseService(const utility::ILogger& logger, boost::asio::thread_pool& thread_pool, const SqlInfo& sql_info);
    ~DatabaseService();

    bool Start();
    void Stop();

    boost::mysql::connection_pool& connection_pool() const;

private:
    const utility::ILogger& logger_;
    boost::asio::thread_pool& thread_pool_;

    const SqlInfo& sql_info_;

    std::shared_ptr<boost::mysql::connection_pool> connection_pool_;
};

} // namespace dev::database
