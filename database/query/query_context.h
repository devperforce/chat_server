#pragma once

#include <memory>

namespace dev::utility {
class ILogger;
} // namespace dev

namespace boost::mysql {
class connection_pool;
} // namespace boost::mysql

namespace dev::database {

class IQueryContext {
public:
    virtual const utility::ILogger& logger() const = 0;
    virtual std::shared_ptr<boost::mysql::connection_pool> connection_pool() const = 0;
};

} // namespace dev::database
