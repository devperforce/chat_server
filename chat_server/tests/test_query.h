#pragma once
#include <boost/core/noncopyable.hpp>

namespace dev::database {
class IQueryContext;
} // namespace dev::database

namespace dev::chat_server {

class TestQuery : boost::noncopyable {
public:
    explicit TestQuery(database::IQueryContext& query_context);

    bool CheckPrepareStatement() const;


private:
    database::IQueryContext& query_context_;
};

} // namespace dev::chat_server
