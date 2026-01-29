#include "pch.h"
#include "tests/test_query.h"

#include "utility/log/logger.h"
#include "database/query/query_context.h"
#include "database/query/query_execution.h"

namespace dev::chat_server {

TestQuery::TestQuery(database::IQueryContext& query_context)
    : query_context_(query_context) {
}

void TestQuery::CheckPrepareStatement() const {
    /*
    database::ExecuteAsync(
        query_context_.connection_pool(),
        [tag = "test1234"](boost::mysql::pooled_connection& conn) -> boost::asio::awaitable<void> {
            boost::mysql::statement stmt = co_await conn->async_prepare_statement(
                "INSERT INTO tracking_play_data (user_uid, tag) VALUES (?, ?)",
                boost::asio::use_awaitable
            );
            boost::mysql::results result;
            for (int32_t index = 0; index < 10; ++index) {
                auto user_uid = index;
                co_await conn->async_execute(stmt.bind(user_uid, tag), result, boost::asio::use_awaitable);
            }
        },
        [&](const std::exception& e) {
            query_context_.logger().LogError("[TestQuery] Fail to execute prepare statement. exception: {}", e.what());
        }
    );
    */
}

} // namespace dev::chat_server
