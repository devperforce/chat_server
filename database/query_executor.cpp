#include "pch.h"
#include "database/query_executor.h"

#include "database/query.h"

namespace dev::database {

void ExecuteAsync(std::shared_ptr<QueryAsyncBase> query) {
    boost::asio::co_spawn(query->get_conn_pool()->get_executor(), 
        [query]() -> boost::asio::awaitable<void> {
        try {
            auto conn = co_await query->get_conn_pool()->async_get_connection(boost::asio::use_awaitable);
            co_await query->Execute(conn);
        } catch (const std::exception& e) {
            query->OnError(e);
        }
    }, 
        boost::asio::detached
    );
}



} // namespace dev::database
