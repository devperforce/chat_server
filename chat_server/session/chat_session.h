#pragma once

#include <boost/asio.hpp>
#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"
#include "protobuf/protobuf_session.h"

namespace dev::chat_server {

class ChatServer;
class User;

class ChatSession final : public protobuf::ProtobufSession {
public:
    ChatSession(
        const ChatServer& chat_server,
        boost::asio::ip::tcp::socket&& socket,
        const utility::ILogger& logger,
        const network::PacketHandler& packet_handler
    );
    ~ChatSession();

    const ChatServer& chat_server() const;

    std::shared_ptr<User> user() const;
    void set_user(std::shared_ptr<User> user);

    void RecvPing();

private:
    void OnConnected() override;
    void OnDisconnected(const boost::system::error_code& ec) override;
    void OnError(const boost::system::error_code& ec) override;

    void SendPing();

    const ChatServer& chat_server_;
    std::weak_ptr<User> user_;

    boost::asio::steady_timer ping_timer_;
    std::chrono::system_clock::time_point last_ping_time_;

    uint32_t debug_unique_id_ = 0;
};

} // namespace dev::chat_server
