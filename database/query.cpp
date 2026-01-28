#include "pch.h"
#include "database/query.h"

namespace dev::database {
QueryAsyncBase::QueryAsyncBase(const utility::ILogger& logger, std::shared_ptr<boost::mysql::connection_pool> conn_pool)
    : logger_(logger), conn_pool_(std::move(conn_pool)) {

}

QueryAsyncBase::~QueryAsyncBase() {
}

std::shared_ptr<boost::mysql::connection_pool> QueryAsyncBase::get_conn_pool() const {
    return conn_pool_;
}

} // namespace dev::database