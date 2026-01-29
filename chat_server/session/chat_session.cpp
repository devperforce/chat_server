#include "pch.h"
#include "chat_server/session/chat_session.h"

#include "database_service.h"
#include "protobuf/generated/chatting.pb.h"
#include "database/query.h"
#include "database/query_executor.h"
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
        const utility::ILogger& logger,
        std::shared_ptr<boost::mysql::connection_pool> conn_pool,
        std::shared_ptr<ChatSession> chat_session
    ) : QueryAsyncBase(logger, std::move(conn_pool)),
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


        auto user_info_query = std::make_shared<GetUserInfoQuery>(
            logger_,
            chat_server_.database_service().connection_pool(),
            std::static_pointer_cast<ChatSession>(self)
        );

        ExecuteAsync(user_info_query, [&, chat_session = std::static_pointer_cast<ChatSession>(self)](GetUserInfoQuery::ResultType db_user) mutable {
            logger_.LogInfo("[ChatSession] User info retrieved: {} ({})", db_user->nickname, db_user->user_uid);
            CONTROLLER(chat_session).Post([&](ChatSession& chat_session) {
                chat_session.SendPing(); 
            });
        });
        
        /*
        database::ExecuteAsync(
            chat_server_.database_service().connection_pool(),
            [&, self = shared_from_this(), target_uids = std::vector<int>{ 10, 11, 12 }, &logger = logger_] (boost::mysql::pooled_connection& conn) -> boost::asio::awaitable<void>{
                logger.LogDebug("database::ExecuteAsync<DBUser>");

                boost::mysql::static_results<DBUser> results;
                co_await conn->async_execute(boost::mysql::with_params("SELECT user_uid, nickname FROM user WHERE user_uid IN ({})", target_uids), results);

                for (const auto& user : results.rows()) {
                    const auto user_uid = user.user_uid;
                    const std::u16string utf16_nickname = boost::locale::conv::utf_to_utf<char16_t>(user.nickname);
                    //std::string utf8_nickname = boost::locale::conv::utf_to_utf<char>(utf16_nickname);
                    self->logger().LogInfo("Found user: {} ({})", user.nickname, user_uid);
                }

               SendPing();
            },
            [self = shared_from_this()](const std::exception& e) {
                self->logger().LogError("[ChatSession] Fail to execute query. exception: {}", e.what());
                self->Close();
        });
        */
    });

    std::string tag = "asdasdasdasd";
    database::ExecuteAsync(
        chat_server_.database_service().connection_pool(),
        [self = shared_from_this(), tag](boost::mysql::pooled_connection& conn) -> boost::asio::awaitable<void> {
     
            boost::mysql::statement stmt = co_await conn->async_prepare_statement(
                "INSERT INTO tracking_play_data (user_uid, tag) VALUES (?, ?)",
                boost::asio::use_awaitable
            );
            boost::mysql::results result;
            for (int32_t index = 0; index < 10; ++index) {
                co_await conn->async_execute(stmt.bind(10, tag), result, boost::asio::use_awaitable);
            }
        },
            [self = shared_from_this()](const std::exception& e) {
            self->logger().LogError("[ChatSession] Fail to execute query. exception: {}", e.what());
            self->Close();
        }
    );

    last_ping_time_ = refresh_ping_time;
    const auto req = std::make_shared<chat::PingReq>();
    Send(req);
}

} // namespace dev::chat_server
