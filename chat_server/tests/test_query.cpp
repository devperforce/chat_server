#include "pch.h"
#include "tests/test_query.h"

#include "utility/log/logger.h"
#include "database/query/query_context.h"
#include "database/query/query_execution.h"

namespace dev::chat_server {

struct UserInfo {
    std::string nickname;
    uint64_t gold;
};
BOOST_DESCRIBE_STRUCT(UserInfo, (), (nickname, gold))

TestQuery::TestQuery(database::IQueryContext& query_context)
    : query_context_(query_context) {
}

bool TestQuery::CheckExecuteWithParams() const {
    const uint64_t test_user_uid = 10;

    // 테스트 용도로 future를 사용하지만 권장하지 않음
    auto fut_result = database::ExecuteAsyncFuture<UserInfo>(
        query_context_,
        boost::mysql::with_params(
            "SELECT nickname, gold FROM user WHERE user_uid = {}",
            test_user_uid
        )
    );

    const auto& logger = query_context_.logger();

    try {
        if (fut_result.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
            logger.LogError("[TestQuery::CheckPrepareStatement] timed out");
            return false;
        }

        auto results = fut_result.get();
        for (const auto& row : results.rows()) {
            UNREFERENCED_PARAMETER(row);
        }
        logger.LogDebug("[TestQuery::CheckPrepareStatement] Passed successfully");

        return true;
    } catch (const std::exception& e) {
        logger.LogError(
            "[TestQuery::CheckPrepareStatement] Failed with exception. error={}",
            e.what()
        );
        return false;
    } catch (...) {
        logger.LogError("[TestQuery::CheckPrepareStatement] Failed with exception.");
        return false;
    }
}

} // namespace dev::chat_server
