#include "pch.h"
#include "chat_server/session/chat_session.h"

#include "protobuf/generated/chatting.pb.h"
#include "chat_server/chat_server.h"
#include "chat_server/user_detail/user.h"

namespace dev::chat_server {

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

struct DBUser {
    uint64_t user_uid;
    std::string nickname;
};
BOOST_DESCRIBE_STRUCT(DBUser, (), (user_uid, nickname))

boost::asio::awaitable<DBUser> Test(const utility::ILogger& logger, std::shared_ptr<boost::mysql::connection_pool> db_conn_pool) {
    auto conn = co_await db_conn_pool->async_get_connection(boost::asio::use_awaitable);
    boost::mysql::static_results<DBUser> result;

    uint64_t user_uid = 51;

    co_await conn->async_execute(
        boost::mysql::with_params("SELECT * FROM user WHERE user_uid = {}", user_uid),
        result,
        boost::asio::use_awaitable
    );

    logger.LogDebug("called Test");

    DBUser u{};
    if (result.has_value()) {
        u = result.rows()[0]; // 첫 번째 행을 User 객체로 바로 가져옴
    }

    co_return u;
}

boost::asio::awaitable<void> Test2(const utility::ILogger& logger, std::shared_ptr<boost::mysql::connection_pool> db_conn_pool) {
    auto conn = co_await db_conn_pool->async_get_connection(boost::asio::use_awaitable);
    
    uint64_t user_uid = 51;
    std::string tag = "32423424234234";

    boost::mysql::results result;
    // 이 구문이 컴파일되려면 boost/mysql/with_params.hpp 가 포함되어야 합니다.
    co_await conn->async_execute(
        boost::mysql::with_params(
            "INSERT INTO tracking_play_data (user_uid, tag) VALUES ({}, {})", 
            user_uid, 
            tag
        ),
        result,
        boost::asio::use_awaitable
    );
    logger.LogDebug("called Test2");
}

#include <expected>
#include <string>

boost::asio::awaitable<std::expected<DBUser, std::string>> Test(const utility::ILogger& logger, std::shared_ptr<boost::mysql::connection_pool> db_conn_pool) {
    auto conn = co_await db_conn_pool->async_get_connection(boost::asio::use_awaitable);
    boost::mysql::static_results<DBUser> result;

    uint64_t user_uid = 51;

    co_await conn->async_execute(
        boost::mysql::with_params("SELECT * FROM user WHERE user_uid = {}", user_uid),
        result,
        boost::asio::use_awaitable
    );

    logger.LogDebug("called Test");

    DBUser u{};
    if (result.has_value()) {
        u = result.rows()[0]; // 첫 번째 행을 User 객체로 바로 가져옴
    }

    co_return u;
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

        SendPing();
    });

    auto db_conn_pool = chat_server_.db_conn_pool();

    auto self = shared_from_this();

    boost::asio::co_spawn(
        db_conn_pool->get_executor(),
        [db_conn_pool, self] -> boost::asio::awaitable<void> {
            return Test2(self->logger(), db_conn_pool);
        },
        [&, self](std::exception_ptr e) {
            if (e) {
                try {
                    std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    self->logger().LogError("Coroutine finished with exception: {}", ex.what());
                }
            } else {
                last_ping_time_ = refresh_ping_time;            
                const auto req = std::make_shared<chat::PingReq>();
                Send(req);
                self->logger().LogDebug("Coroutine finished successfully");
            }
        }
    );


    /*
    boost::asio::co_spawn(
        db_conn_pool->get_executor(),
        [db_conn_pool, self] {
            return Test(self->logger(), db_conn_pool);
        },
        [&, self, last_ping_time = refresh_ping_time](std::exception_ptr e, DBUser user) {
            if (e) {
                try {
                    std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    // 이제 정확한 에러 원인(ex.what())을 볼 수 있습니다.
                    self->logger().LogError("[ChatSession] DB error: {}", ex.what());
                }
                self->Close();
                return;
            }

            logger_.LogDebug("[ChatSession] DBTest. user_uid: {}", user.user_uid);
            last_ping_time_ = refresh_ping_time;            
            const auto req = std::make_shared<chat::PingReq>();
            Send(req);
        }
    );
    */
}

} // namespace dev::chat_server
