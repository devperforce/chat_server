#include "pch.h"
#include "tests/test_query.h"

#include "utility/log/logger.h"
#include "database/query/query_context.h"
#include "database/query/query_execution.h"

namespace dev::chat_server {


struct TestInfo {
    int32_t a;
    float b;
};
BOOST_DESCRIBE_STRUCT(TestInfo, (), (a, b))


TestQuery::TestQuery(database::IQueryContext& query_context)
    : query_context_(query_context) {
}

bool TestQuery::CheckPrepareStatement() const {
    auto params = boost::mysql::with_params("select * from users where id = ?", 1);

    // 1. co_spawn을 통해 awaitable을 future로 변환
    std::future<boost::mysql::static_results<TestInfo>> fut = boost::asio::co_spawn(
        query_context_.connection_pool()->get_executor(),
        ExecuteAsync<TestInfo>(query_context_, params),
        boost::asio::use_future
    );

    try {
        auto results = fut.get();
        for (const auto& row : results.rows()) {
            // 비즈니스 로직
            (void)row;
        }
        return true;
    } catch (const std::exception& e) {
        query_context_.logger().LogError(
            "[TestQuery::CheckPrepareStatement] failed with exception. error={}", e.what()
        );
        return false;
    } catch (...) {
        query_context_.logger().LogError("[TestQuery::CheckPrepareStatement] failed with exception.");
        return false;
    }
}

} // namespace dev::chat_server
