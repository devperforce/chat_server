#include "pch.h"
#include "chat_server/session/chat_session.h"

#include "protobuf/generated/chatting.pb.h"
#include "database/database_service.h"
#include "database/query/query_execution.h"
#include "database/query/query_async.h"
#include "chat_server/chat_server.h"
#include "chat_server/user_detail/user.h"

namespace dev::chat_server {

struct DBUser {
    uint64_t user_uid;
    std::string nickname;
};
BOOST_DESCRIBE_STRUCT(DBUser, (), (user_uid, nickname))

class GetUserInfoQuery final : public database::QueryAsyncBase<DBUser> {
public:
    GetUserInfoQuery(
        const database::IQueryContext& context,
        std::shared_ptr<ChatSession> chat_session
    ) : QueryAsyncBase(context),
        chat_session_(std::move(chat_session)) {
    }
    ~GetUserInfoQuery() = default;

    boost::asio::awaitable<ResultType> Execute(boost::mysql::pooled_connection& conn) override {
        boost::mysql::statement stmt = co_await conn->async_prepare_statement(
            "SELECT * FROM user WHERE user_uid = ?",
            boost::asio::use_awaitable
        );
        boost::mysql::static_results<DBUser> results;

        co_await conn->async_execute(stmt.bind(10), results, boost::asio::use_awaitable);

        for (const auto& user : results.rows()) {
            const auto user_uid = user.user_uid;
            const std::u16string utf16_nickname = boost::locale::conv::utf_to_utf<char16_t>(user.nickname);
            //std::string utf8_nickname = boost::locale::conv::utf_to_utf<char>(utf16_nickname);
            co_return std::make_shared<DBUser>(user_uid, user.nickname);
        }

        throw std::runtime_error("User not found");
    }

    void OnError(const std::exception& e) const override {
        logger_.LogError("[GetUserInfoQuery] Fail to execute query. exception: {}", e.what());
    }

private:
    std::shared_ptr<ChatSession> chat_session_;
};

ChatSession::ChatSession(
    const ChatServer& chat_server,
    boost::asio::ip::tcp::socket&& socket,
    const utility::ILogger& logger,
    const network::PacketHandler& packet_handler
) : ProtobufSession(chat_server.io_context(), std::move(socket), logger, packet_handler),
    chat_server_(chat_server),
    ping_timer_(strand_) {
}

ChatSession::~ChatSession() {
    logger_.LogInfo("[ChatSession] ~ChatSession");
}

void ChatSession::OnConnected() {
    logger_.LogInfo("[ChatSession] OnConnected");
    SendPing();
}

void ChatSession::OnDisconnected(const boost::system::error_code& ec) {
    logger_.LogInfo("[ChatSession] OnDisconnected, {}", ec.what());
    BOOST_ASSERT(TestSynchronize());
    
    auto user = user_.lock();
    if (user != nullptr) {
        CONTROLLER(user).Post([] (User& user) {
            user.OnDisconnected();
        });
    }
}

void ChatSession::OnError(const boost::system::error_code& ec) {
    logger_.LogError("[ChatSession] OnError, {}", ec.what());
}

const ChatServer& ChatSession::chat_server() const {
    return chat_server_;
}

std::shared_ptr<User> ChatSession::user() const {
    BOOST_ASSERT(TestSynchronize());

    return user_.lock();
}

void ChatSession::set_user(std::shared_ptr<User> user) {
    BOOST_ASSERT(TestSynchronize());
    if (user == nullptr) {
        ping_timer_.cancel();
    }
    user_ = user;
}

void ChatSession::RecvPing() {
    last_ping_time_ = std::chrono::system_clock::now();
}

void ChatSession::SendPing() {
    static constexpr auto kWaitDuration = std::chrono::seconds(10);

    auto refresh_ping_time = std::chrono::system_clock::now();

    ping_timer_.expires_after(kWaitDuration);
    ping_timer_.async_wait([this, self = shared_from_this(), start_time = refresh_ping_time](const boost::system::error_code& ec) {

        BOOST_ASSERT(TestSynchronize());
        if (ec || boost::asio::steady_timer::clock_type::now() < ping_timer_.expiry()) {
            logger_.LogDebug("[ChatSession] ping timer cancel!, debug_unique_id: {}", debug_unique_id_);
            return;
        }

        if (last_ping_time_ <= start_time) {
            Close();
            return;
        }


        // Example async DB usage with correct callback signature; adjust as needed
        DBUser a;
        a.user_uid = 10;
        database::ExecuteAsync( // <DBUser> 생략 가능!
            chat_server_.database_service().connection_pool(),
            boost::mysql::with_params("SELECT * FROM user WHERE user_uid = {}", a.user_uid),
            [this, self = shared_from_this()](boost::system::error_code ec, boost::mysql::static_results<DBUser> results) { // 여기서 타입을 명시
                if (ec) {
                    logger_.LogError("error_code: {}", ec.what());
                    return;
                }

                for (const auto& user : results.rows()) {
                    logger_.LogDebug("User: {} ({})", user.nickname, user.user_uid);
                }

                SendPing();
            }
        );

    });

    last_ping_time_ = refresh_ping_time;
    const auto req = std::make_shared<chat::PingReq>();
    Send(req);

    database::ExecuteAsync(
        chat_server_.database_service().connection_pool(),
        [this, self = shared_from_this()](boost::mysql::pooled_connection& conn) -> boost::asio::awaitable<void> {
            // Prepare the statement once
            boost::mysql::statement stmt = co_await conn->async_prepare_statement(
                "SELECT * FROM user WHERE user_uid = ?",
                boost::asio::use_awaitable
            );

            boost::mysql::static_results<DBUser> results;
         
            co_await conn->async_execute(
                stmt.bind(10),
                results,
                boost::asio::use_awaitable
            );

            if (results.rows().empty()) {
                co_return;
            }

            for (const auto& user : results.rows()) {
                logger_.LogDebug("User2222: {} ({})", user.nickname, user.user_uid);
            }
        },
        [this, self = shared_from_this()](const std::exception& e) {
            this->Close();
        }
    );

}

} // namespace dev::chat_server
