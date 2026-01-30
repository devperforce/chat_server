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

class UserQuery : public database::QueryAsyncBase {
public:
    explicit UserQuery(const database::IQueryContext& query_context)
        : QueryAsyncBase(query_context) {
    }

    void GetUserInfo(int user_id, std::function<void(boost::system::error_code, boost::mysql::static_results<DBUser>)> callback) {
        ExecuteAsyncInternal(
            boost::mysql::with_params("SELECT * FROM user WHERE user_uid = {}", user_id),
            std::move(callback)
        );
    }
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
    if (ec == boost::asio::error::eof ||
        ec == boost::asio::error::connection_reset ||
        ec == boost::asio::error::operation_aborted) {
        logger_.LogDebug("[ChatSession] OnError (connection closed), {}", ec.message());
    } else if (ec.category() == boost::system::system_category()) {
        logger_.LogWarning("[ChatSession] OnError (system), {} ({})", ec.message(), ec.value());
    } else {
        logger_.LogError("[ChatSession] OnError (unexpected), {} ({})", ec.message(), ec.value());
    }
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

        DBUser a;
        a.user_uid = 10;
        database::ExecuteAsync(
            chat_server_.database_service(),
            boost::mysql::with_params("SELECT * FROM user2 WHERE user_uid = {}", a.user_uid),
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

    UserQuery(chat_server_.database_service())
        .GetUserInfo(10, [this, self = shared_from_this()](boost::system::error_code ec, boost::mysql::static_results<DBUser> results) {
            if (ec) {
                logger_.LogDebug("[ChatSession] error_code: {}", ec.message());
                return;
            }
            for (const auto& user : results.rows()) {
                logger_.LogDebug("[ChatSession] User: {} ({})", user.nickname, user.user_uid);
            }
    });

}

} // namespace dev::chat_server
