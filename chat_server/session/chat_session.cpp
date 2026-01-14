#include "pch.h"
#include "chat_server/session/chat_session.h"

#include "protobuf/generated/chatting.pb.h"
#include "chat_server/chat_server.h"
#include "user_detail/user.h"

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

void ChatSession::SendPing() {
    static constexpr auto kWaitDuration = std::chrono::seconds(10);

    auto refresh_ping_time = std::chrono::system_clock::now();

    ping_timer_.expires_from_now(kWaitDuration);
    ping_timer_.async_wait([this, self = shared_from_this(), start_time = refresh_ping_time](const boost::system::error_code& ec) {
        BOOST_ASSERT(TestSynchronize());
        if (ec || boost::asio::steady_timer::clock_type::now() < ping_timer_.expires_at()) {
            logger_.LogDebug("[ChatSession] ping timer cancel!, debug_unique_id: {}", debug_unique_id_);
            return;
        }

        if (last_ping_time_ <= start_time) {
            Close();
            return;
        }

        SendPing();
    });

    last_ping_time_ = refresh_ping_time;
    const auto req = std::make_shared<chat::PingReq>();
    Send(req);
}

} // namespace dev::chat_server
