#pragma once

#include <boost/core/noncopyable.hpp>

#include "database/query/query_context.h"

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {

struct SqlInfo;

class DatabaseService final : boost::noncopyable , public IQueryContext {
public:
    DatabaseService(const utility::ILogger& logger, boost::asio::thread_pool& thread_pool, const SqlInfo& sql_info);
    ~DatabaseService();

    bool Start();
    void Stop();

    const utility::ILogger& logger() const override; 
    std::shared_ptr<boost::mysql::connection_pool> connection_pool() const override;

private:
    const utility::ILogger& logger_;
    boost::asio::thread_pool& thread_pool_;

    const SqlInfo& sql_info_;

    std::shared_ptr<boost::mysql::connection_pool> connection_pool_;
};

} // namespace dev::database
