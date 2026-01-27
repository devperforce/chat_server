#pragma once

#include <boost/core/noncopyable.hpp>
#include <chrono>

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::utility {
class ILogger;
} // namespace dev

namespace dev::database {

class ConnectionValidator : boost::noncopyable {
public:
    explicit ConnectionValidator(const utility::ILogger& logger);

    bool Check(
        boost::mysql::connection_pool& db_conn_pool,
        std::chrono::steady_clock::duration wait_time = std::chrono::seconds(5)
    ) const;

private:
    const utility::ILogger& logger_;
};

} // namespace dev::database
